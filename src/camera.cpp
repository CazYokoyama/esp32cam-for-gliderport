/*
 * camera.cpp
 * Copyright(c) 2021 by Caz Yokoyama, caz@caztech.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Arduino.h>
#include "esp_camera.h"
#include "time.h"
#include "config.h"
#include "wifi.hpp"
#include "camera.hpp"

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

extern struct tm timeinfo;

esp_err_t
camera_init()
{
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers
  if(psramFound()){
    /*
      HD is the maximum size

      FRAMESIZE_VGA,      // 640x480 - works
      FRAMESIZE_SVGA,     // 800x600 - fail in saving in SD card
      FRAMESIZE_XGA,      // 1024x768 - works
      FRAMESIZE_HD,       // 1280x720 - Maximum resolution on AI Thinker
      FRAMESIZE_SXGA,     // 1280x1024 - can't allocate matrix3du
      FRAMESIZE_UXGA,     // 1600x1200 - can't allocate matrix3du
     */
    config.frame_size = FRAMESIZE_HD;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
    return err;

  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

  delay(2000);

  return err;
}

#include "img_converters.h"
#include <fb_gfx.h>
#include <fr_forward.h>

static void
rgb_print(dl_matrix3du_t *image_matrix,
          uint32_t x, uint32_t y,
          uint32_t color, const char *str)
{
    fb_data_t fb;
    fb.width = image_matrix->w;
    fb.height = image_matrix->h;
    fb.data = image_matrix->item;
    fb.bytes_per_pixel = 3;
    fb.format = FB_BGR888;
    fb_gfx_print(&fb, x, y, color, str);
}

uc_t
get_average_brightness(dl_matrix3du_t *image_matrix)
{
  int x, y;
  unsigned long w, h;
  uc_t *p = image_matrix->item;

  w = 0;
  for (y = 0; y < image_matrix->h; y++) {
    h = 0;
    for (x = 0; x < image_matrix->w; x++) {
      h += *p++;
    }
    w += h / image_matrix->w;
  }
  return (w / image_matrix->h);
}

dl_matrix3du_t *
acquire_rgb888()
{
    /* capture photo */
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return NULL;
    }

    /* convert to rgb888 format */
    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
    if (!image_matrix) {
        Serial.println("dl_matrix3du_alloc failed");
        return NULL;
    }
    fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);

    esp_camera_fb_return(fb);

    return image_matrix;
}

void
release_rgb888(dl_matrix3du_t *image_matrix)
{
    dl_matrix3du_free(image_matrix);
}

/*
  capture Photo, overlay caption and timestamp
*/
void
capturePhoto(uint8_t **_jpg_buf, size_t *_jpg_buf_len)
{
    dl_matrix3du_t *image_matrix = acquire_rgb888();
    uc_t brightness = get_average_brightness(image_matrix);
    Serial.printf("%u ", brightness);

    /* overlay caption */
    rgb_print(image_matrix,
              5, image_matrix->h - 25,
              0x00ffffff, caption.c_str());

    /* overlay timestamp */
    struct tm timeinfo;
    memset(&timeinfo, 0, sizeof(timeinfo));
    if (getLocalTime(&timeinfo)) {
        char timeStringBuff[50]; //50 chars should be enough
        strftime(timeStringBuff, sizeof(timeStringBuff), "%Y/%m/%d %H:%M:%S", &timeinfo);
        rgb_print(image_matrix,
                  image_matrix->w - strlen(timeStringBuff) * 14 - 5, image_matrix->h - 25,
                  0x00ffffff, timeStringBuff);
    } else
        Serial.println("Failed to obtain time");

    /* convert rgb888 to jpeg */
    bool jpeg_converted = fmt2jpg(image_matrix->item, image_matrix->w*image_matrix->h*3,
                                  image_matrix->w, image_matrix->h,
                                  PIXFORMAT_RGB888, 90,
                                  _jpg_buf, _jpg_buf_len);
    if (!jpeg_converted) {
        Serial.println("Failed to convert to jpeg");
        return;
    }
    release_rgb888(image_matrix);
}
