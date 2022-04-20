/*
 * wifi.hpp
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
#ifndef WIFI_HPP
#define WIFI_HPP

#include <WiFi.h>
#include <WiFiMulti.h>
#include <esp_wifi.h>

extern WiFiMulti *wifiMulti;

static const int8_t ESP32_dB_to_power_level[] = {
    -4, /* -1   dB, #0 */
    8,  /* 2    dB, #1 */
    20, /* 5    dB, #2 */
    28, /* 7    dB, #3 */
    34, /* 8.5  dB, #4 */
    44, /* 11   dB, #5 */
    52, /* 13   dB, #6 */
    60, /* 15   dB, #7 */
    68, /* 17   dB, #8 */
    74, /* 18.5 dB, #9 */
    76, /* 19   dB, #10 */
    78  /* 19.5 dB, #11 */
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
enum
{
    WIFI_TX_POWER_MIN = 0,  /* 0  dBm */
    WIFI_TX_POWER_MED = ARRAY_SIZE(ESP32_dB_to_power_level) / 2,
    WIFI_TX_POWER_MAX = ARRAY_SIZE(ESP32_dB_to_power_level) - 1
};

void wifi_ap_setup();
WiFiMulti *wifi_setup();
void wifi_close();

#endif /* WIFI_HPP */
