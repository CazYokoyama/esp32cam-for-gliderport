/*
  Upload an image taken by a camera by HTTP.
  Expect /mobilewebcam-serverside such as https://github.com/cyberic99/mobilewebcam-serverside
  on HTTP server.
  Copyright(c) 2021 by Caz Yokoyama, caz@caztech.com

  Based on the software creditted as below and heavyly modified.

  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-post-image-photo-server/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include "soc/rtc_cntl_reg.h"
#include "src/Web.h"
#include "src/config.h"
#include "src/wifi.hpp"
#include "src/camera.hpp"

#define uS_TO_M_FACTOR (60 * 1000000)  /* Conversion factor for micro seconds to minutes */

static bool webui;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);

  Serial.println("Start or wake up from deep sleep");

  esp_err_t err = camera_init();
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  if (read_config() && wifi_setup()) {
      //init and get the time
      configTime(gmtOffset_hour * 3600,
                 daylightOffset_hour * 3600,
                 ntpServer.c_str());
      struct tm timeinfo;
      memset(&timeinfo, 0, sizeof(timeinfo));
      getLocalTime(&timeinfo);
      if (timeinfo.tm_hour * 100 + timeinfo.tm_min <= start_upload ||
          end_upload < timeinfo.tm_hour * 100 + timeinfo.tm_min) {
          wifi_close();
          ulong deep_sleep_min;
          if (end_upload > start_upload) {
              deep_sleep_min = 24 * 60 - ((end_upload / 100) * 60 + (end_upload % 100));
              deep_sleep_min += (start_upload / 100) * 60 + (start_upload % 100);
          } else { /* end_upload < start_upload */
              deep_sleep_min = (start_upload / 100) * 60 + (start_upload % 100) -
                              ((end_upload / 100) * 60 + (end_upload % 100));
          }
          Serial.printf("Go to deep sleep until %02d:%02d\n",
                         start_upload / 100, start_upload % 100);
          esp_sleep_enable_timer_wakeup(deep_sleep_min * uS_TO_M_FACTOR);
          esp_deep_sleep_start();
      }
  } else {
      /* set up AP because we don't know SSID/password or can't connect. */
      wifi_ap_setup();
  }
  Web_setup();
}

void loop() {
    Web_loop();
}
