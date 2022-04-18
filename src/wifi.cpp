/*
 * wifi.cpp
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
#include <WiFiMulti.h>
#include "wifi.hpp"
#include "config.h"

WiFiClient client;
WiFiMulti *wifiMulti;

/**
 * Default WiFi connection information.
 *
 */
String host_name PROGMEM = "ESP32CAM";
IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
String station_ssid PROGMEM = "ognbase";
const char* ap_default_psk PROGMEM = "987654321"; ///< Default PSK.

static void
ESP32_WiFi_setOutputPower(int dB)
{
    if (dB > 20)
        dB = 20;

    if (dB < 0)
        dB = 0;

    esp_wifi_set_max_tx_power(ESP32_dB_to_power_level[dB]);
}
void
wifi_ap_setup()
{
    WiFi.setHostname(host_name.c_str());
    WiFi.mode(WIFI_AP);
    ESP32_WiFi_setOutputPower(WIFI_TX_POWER_MED); // 13 dB
    delay(10);
    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
        Serial.println(F("can't configure soft-AP"));
        return;
    }
    Serial.print(host_name);
    Serial.print(F(" is waiting for connection at "));
    Serial.println(WiFi.softAPIP());
}

WiFiMulti *
wifi_setup()
{
  wifiMulti = new WiFiMulti();
  for (int i = 0; i < N_APs; i++) {
    if (strcmp(wifi_ssid[i].c_str(), "xxxxxxx") != 0) {
      Serial.printf("%s() wifi[%d]: %s/%s\n", __func__,
                    i, wifi_ssid[i].c_str(), wifi_pass[i].c_str());
      wifiMulti->addAP(wifi_ssid[i].c_str(), wifi_pass[i].c_str());
    }
  }
  for (int n = 0; n < 10; n++) { /* retry */
    if (wifiMulti->run() == WL_CONNECTED) {
      ESP32_WiFi_setOutputPower(wifiTxPower);
      delay(10);
      Serial.print("Web server "); Serial.print(WiFi.localIP());
      Serial.print(" is up through "); Serial.println(WiFi.SSID());
      return wifiMulti;
    }
    delay(500);
  }
  /* Serial.println("All APs does not work"); */
  return wifiMulti;
}

void
wifi_close()
{
    esp_wifi_stop();
    delete wifiMulti;
}
