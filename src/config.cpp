/*
   CONFIG.cpp
 * Copyright(c) 2021 by Caz Yokoyama, caz@caztech.com
   Copyright (C) 2020 Manuel Rösel

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "EEPROM.h"
#include "config.h"
#include "SPIFFS.h"

#include <ArduinoJson.h>



//wifi
String wifi_ssid[N_APs] = {
         "xxxxxxx",
         "xxxxxxx",
         "xxxxxxx",
         "xxxxxxx",
         "xxxxxxx"
};
String wifi_pass[N_APs] = {
         "xxxxxxx",
         "xxxxxxx",
         "xxxxxxx",
         "xxxxxxx",
         "xxxxxxx"
};
String ntpServer = "pool.ntp.org";
int8_t wifiTxPower = 0;      /* minimum */
uc_t dark_threshold = 25;
long gmtOffset_hour = -8;    /* PDT: -8 */
int  daylightOffset_hour = 1; /* 1 hour */
String caption = "North Plains Glider port";
int  timerInterval = 60;       /* 1min * 60 = 60 sec */
int  start_upload = 0500; /* start hour of taking image */
int  end_upload = 2200;   /* start hour of taking image */

bool read_config(void)
{
    const size_t        capacity = 2048;
    DynamicJsonDocument baseConfig(capacity);
    JsonObject          obj;
    File configFile;

    const char *config_files[6] = { "/config.json",
                                    "/index.html",
                                    "/update.html",
                                    "/style.css",
                                    "/key.bin",
                                    "/iv.bin"};

    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return false;
    }

    if (SPIFFS.exists(config_files[0])){
      configFile = SPIFFS.open(config_files[0]);
      if (!configFile)
      {
          Serial.println(F("Failed to open config.json."));
          return false;
      }
    }
    else{
      Serial.println(F("config.json doesnt exist, please upload config.json"));
      return(false);
    }

    DeserializationError error = deserializeJson(baseConfig, configFile);

    if (error)
    {
        Serial.println(F("Failed to parse json file, using default configuration"));
        Serial.println(error.f_str());
        configFile.close();
        return false;
    }
    else
    {
        obj = baseConfig.as<JsonObject>();
        configFile.close();
    }

    if (!obj.containsKey(F("wifi"))){
        //Serial.println("no wifi confgiuration found, return setup mode");
        configFile.close();
        return false;
    }
    else
    {
        for (int i = 0; i < N_APs; i++) {
            String ssid = obj["wifi"]["ssid"][i].as<String>();
            String pass = obj["wifi"]["pass"][i].as<String>();
            if (ssid != String("xxxxxxx")) {
                Serial.printf("%s() wifi[%d]: %s/%s\n", __func__, i,
                              ssid.c_str(), pass.c_str());
                if (pass != String("hidepass")) {
                    wifi_ssid[i] = ssid;
                    wifi_pass[i] = pass;
                }
            }
        }
    }

    if (!obj.containsKey(F("upload"))) {
        configFile.close();
        return false;
    }
    ntpServer          = obj["upload"]["ntpServer"].as<String>();
    wifiTxPower        = obj["upload"]["wifiTxPower"];
    gmtOffset_hour      = obj["upload"]["gmtOffset_hour"];
    daylightOffset_hour = obj["upload"]["daylightOffset_hour"];
    caption            = obj["upload"]["caption"].as<String>();
    timerInterval      = obj["upload"]["timerInterval"];
    start_upload       = obj["upload"]["start_upload"];
    end_upload         = obj["upload"]["end_upload"];

    return true;
}

bool save_config(void)
{
    const size_t        capacity = 2048;
    DynamicJsonDocument baseConfig(capacity);
    JsonObject          obj;

    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return false;
    }


    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile)
    {
        Serial.println(F("Failed to open config.json readonly"));
        return false;
    }

    DeserializationError error = deserializeJson(baseConfig, configFile);

    if (error)
    {
        Serial.println(F("Failed to read file, using default configuration, format spiffs"));
        configFile.close();
        SPIFFS.format();
        return false;
    }
    else
    {
        obj = baseConfig.as<JsonObject>();
        configFile.close();
    }

    configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println(F("Failed to open config.json write operation"));
        return false;
    }

    for (int i = 0; i < N_APs; i++) {
        if (strcmp(wifi_ssid[i].c_str(), "xxxxxxx") != 0)
            Serial.printf("%s() wifi[%d]: %s/%s\n", __func__, i,
                          wifi_ssid[i].c_str(), wifi_pass[i].c_str());
        obj["wifi"]["ssid"][i] = wifi_ssid[i];
        obj["wifi"]["pass"][i] = wifi_pass[i];
    }
    obj["upload"]["ntpServer"]          = ntpServer;
    obj["upload"]["wifiTxPower"]        = wifiTxPower;
    obj["upload"]["gmtOffset_hour"]      = gmtOffset_hour;
    obj["upload"]["daylightOffset_hour"] = daylightOffset_hour;
    obj["upload"]["caption"]            = caption;
    obj["upload"]["timerInterval"]      = timerInterval;
    obj["upload"]["start_upload"]       = start_upload;
    obj["upload"]["end_upload"]         = end_upload;

    if (serializeJson(obj, configFile) == 0)
        Serial.println(F("Failed to write to file"));

    configFile.close();
    return true;
}
