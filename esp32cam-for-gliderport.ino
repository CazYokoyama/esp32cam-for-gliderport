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

#define mS_TO_S_FACTOR 1000     /* Conversion factor for mili seconds to seconds */

struct tm timeinfo;
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

  if (!read_config())
      goto _ap_;

  if (!wifi_setup()) {
_ap_:
      /* set up AP because we don't know SSID/password or can't connect. */
      wifi_ap_setup();
  }
  Web_setup();
}

void loop()
{
    if (wifiMulti != NULL) { /* STA mode */
      //init and get the time
      configTime(gmtOffset_hour * 3600,
                 daylightOffset_hour * 3600,
                 ntpServer.c_str());
      memset(&timeinfo, 0, sizeof(timeinfo));
      if (!getLocalTime(&timeinfo)) {
          Serial.println("can't obtain local time");
          wifi_close();
          delay(1000);
          ESP.restart();
      }

      sendPhoto();
    }
    Web_loop();
    delay(checkInterval * mS_TO_S_FACTOR); /* delay checkInterval sec */
}
