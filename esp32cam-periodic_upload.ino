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

struct tm timeinfo;
static bool webui;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);

  Serial.println("Wake up from deep sleep");

  read_config();

  esp_sleep_enable_timer_wakeup(timerInterval * uS_TO_S_FACTOR);

  wifi_setup();

  //init and get the time
  configTime(gmtOffset_hour * 3600,
             daylightOffset_hour * 3600,
             ntpServer.c_str());
  memset(&timeinfo, 0, sizeof(timeinfo));
  getLocalTime(&timeinfo);

  camera_init();

#if 0
  if (start_upload <= timeinfo.tm_hour * 100 + timeinfo.tm_min  &&
    timeinfo.tm_hour * 100 + timeinfo.tm_min < end_upload) {
    wifi_close();
    Serial.println("Going to deep sleep");
    Serial.flush();
    esp_deep_sleep_start();
  } else {
#endif
    Serial.print("Web server ");
    Serial.print(my_IP);
    Serial.println(" is up");
    Serial.flush();
    Web_setup();
#if 0
  }
#endif
}

void loop() {
    /* never come here on deep sleep */
    Web_loop();
}
