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

#include <Arduino.h>
#include <WiFiMulti.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "time.h"
#include "src/Web.h"
#include "src/config.h"
#include "src/wifi.hpp"

WiFiClient client;
WiFiMulti *wifiMulti;

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

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

struct tm timeinfo;

const int HS2_DATA2_Pin = 12;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);

  // initialize the pushbutton pin as an input:
  pinMode(HS2_DATA2_Pin, INPUT_PULLUP);

  Serial.println("Wake up from deep sleep");

  read_config();

  esp_sleep_enable_timer_wakeup(timerInterval * uS_TO_S_FACTOR);

  wifi_setup();

  //init and get the time
  configTime(gmtOffset_sec,
             daylightOffset_sec,
             ntpServer.c_str());
  memset(&timeinfo, 0, sizeof(timeinfo));
  getLocalTime(&timeinfo);

  if (digitalRead(HS2_DATA2_Pin) == LOW) {
    Serial.print("Web server ");
    Serial.print(my_IP);
    Serial.println(" is up");
    Serial.flush();
    Web_setup();
  } else {
    if (start_upload <= timeinfo.tm_hour * 100 + timeinfo.tm_min  &&
      timeinfo.tm_hour * 100 + timeinfo.tm_min < end_upload)
      sendPhoto();
    delete wifiMulti;
  }
}

void loop() {
  if (digitalRead(HS2_DATA2_Pin) == LOW)
    Web_loop();
  else {
    Serial.println("Going to deep sleep");
    Serial.flush();
    esp_deep_sleep_start();
  }
}

String sendPhoto() {
  String getAll;
  String getBody;

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
    config.frame_size = FRAMESIZE_SXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  delay(2000);

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }

  Serial.println("Connecting to server: " + serverName);

  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");

    /* create time stamp string */
    char date_time[40];
    snprintf(date_time, sizeof(date_time), "%d-%02d-%02d %02d:%02d:%02d",
      (timeinfo.tm_year)+1900,(timeinfo.tm_mon)+1, timeinfo.tm_mday,
      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    String caption_head = "--ffZk5M3ar7Fodn/1mnJmdX5h+XQbLU07\r\nContent-Disposition: form-data; name=\"caption\"\r\n\r\n";
    String caption_data = caption;
    String caption_tail = "\r\n--ffZk5M3ar7Fodn/1mnJmdX5h+XQbLU07";

    String timestamp_head = "--ffZk5M3ar7Fodn/1mnJmdX5h+XQbLU07\r\nContent-Disposition: form-data; name=\"timestamp\"\r\n\r\n";
    String timestamp_data = String(date_time);
    String timestamp_tail = "\r\n--ffZk5M3ar7Fodn/1mnJmdX5h+XQbLU07";

    String head = "--ffZk5M3ar7Fodn/1mnJmdX5h+XQbLU07\r\nContent-Disposition: form-data; name=\"userfile\"; filename=\"esp32-cam.jpg\"\r\n\r\n";
    String tail = "\r\n--ffZk5M3ar7Fodn/1mnJmdX5h+XQbLU07--\r\n\r\n";

    uint32_t imageLen = fb->len;
    uint32_t extraLen = caption_head.length() + caption_data.length() + caption_tail.length() +
	timestamp_head.length() + timestamp_data.length() + timestamp_tail.length() +
	head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;

    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=ffZk5M3ar7Fodn/1mnJmdX5h+XQbLU07");

    client.println();
    client.print(caption_head);
    client.print(caption_data);
    client.print(caption_tail);

    client.println();
    client.print(timestamp_head);
    client.print(timestamp_data);
    client.print(timestamp_tail);

    client.println();
    client.print(head);

    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0; n<fbLen; n=n+1024) {
      if (n+1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      } else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client.write(fbBuf, remainder);
      }
    }
    client.print(tail);

    esp_camera_fb_return(fb);

    int timoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;

    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(100);
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getAll.length()==0) { state=true; }
          getAll = "";
        } else if (c != '\r')
          getAll += String(c);
        if (state==true)
          getBody += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0)
        break;
    }
    Serial.println();
    client.stop();
    Serial.println(getBody);
  } else {
    getBody = "Connection to " + serverName +  " failed.";
    Serial.println(getBody);
  }
  return getBody;
}
