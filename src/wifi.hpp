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

extern WiFiClient client;

enum
{
    WIFI_TX_POWER_MIN = 0,  /* 0  dBm */
    WIFI_TX_POWER_MED = 10, /* 10 dBm */
    WIFI_TX_POWER_MAX = 18  /* 18 dBm */
};

static const int8_t ESP32_dB_to_power_level[21] = {
    8,  /* 2    dB, #0 */
    8,  /* 2    dB, #1 */
    8,  /* 2    dB, #2 */
    8,  /* 2    dB, #3 */
    8,  /* 2    dB, #4 */
    20, /* 5    dB, #5 */
    20, /* 5    dB, #6 */
    28, /* 7    dB, #7 */
    28, /* 7    dB, #8 */
    34, /* 8.5  dB, #9 */
    34, /* 8.5  dB, #10 */
    44, /* 11   dB, #11 */
    44, /* 11   dB, #12 */
    52, /* 13   dB, #13 */
    52, /* 13   dB, #14 */
    60, /* 15   dB, #15 */
    60, /* 15   dB, #16 */
    68, /* 17   dB, #17 */
    74, /* 18.5 dB, #18 */
    76, /* 19   dB, #19 */
    78  /* 19.5 dB, #20 */
};

void wifi_setup();
void wifi_close();

#endif /* WIFI_HPP */
