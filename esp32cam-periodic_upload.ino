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
    webui = true;
    Serial.print("Web server ");
    Serial.print(my_IP);
    Serial.println(" is up");
    Serial.flush();
    Web_setup();
  } else {
    webui = false;
    if (start_upload <= timeinfo.tm_hour * 100 + timeinfo.tm_min  &&
      timeinfo.tm_hour * 100 + timeinfo.tm_min < end_upload)
      camera_init();
      sendPhoto();
      wifi_close();
  }
}

void loop() {
  if (digitalRead(HS2_DATA2_Pin) == LOW) {
    if (!webui)
      ESP.restart();
    Web_loop();
  } else {
    if (webui)
      ESP.restart();
    Serial.println("Going to deep sleep");
    Serial.flush();
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 0);
    esp_deep_sleep_start();
  }
}
