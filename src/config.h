/*
 * CONFIG.h
 * Copyright(c) 2021 by Caz Yokoyama, caz@caztech.com
 * Copyright (C) 2020 Manuel Rösel
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

#include <string.h>

#ifndef CONFIGHELPER_H
#define CONFIGHELPER_H

#include "dl_lib_matrix3d.h"

#define N_APs 5

extern String wifi_ssid[N_APs];
extern String wifi_pass[N_APs];
extern String ntpServer;
extern int8_t wifiTxPower;
extern uc_t dark_threshold;
extern long gmtOffset_hour;
extern int  daylightOffset_hour;
extern String caption;
extern int  timerInterval;
extern int  start_upload;
extern int  end_upload;

bool save_config(void);

bool read_config(void);

#endif /* CONFIGHELPER_H */
