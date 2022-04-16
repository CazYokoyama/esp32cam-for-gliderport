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

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

static bool webui;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);

  Serial.println("Wake up from deep sleep");

  esp_sleep_enable_timer_wakeup(timerInterval * uS_TO_S_FACTOR);

  esp_err_t err = camera_init();
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  read_config();

  wifi_setup();

  //init and get the time
  configTime(gmtOffset_hour * 3600,
             daylightOffset_hour * 3600,
             ntpServer.c_str());

  Serial.print("Web server ");
  Serial.print(my_IP);
  Serial.println(" is up");
  Serial.flush();
  Web_setup();
}

void loop() {
    /* never come here on deep sleep */
    Web_loop();
}
