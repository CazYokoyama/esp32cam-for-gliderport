/*
 * Web.cpp
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

#include "../git-version.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "config.h"
#include <Update.h>

#define  U_PART U_FLASH

const String version = "0.01-" + String(GIT_VERSION);

// Create AsyncWebServer object on port 80
AsyncWebServer wserver(80);

static const char upload_html[] PROGMEM = "<html>\
                                            <head>\
                                            <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>\
                                            </head>\
                                            <div class = 'upload'>\
                                            <form method = 'POST' action = '/doUpload' enctype='multipart/form-data'>\
                                            <input type='file' name='data'/><input type='submit' name='upload' value='Upload' title = 'Upload Files'>\
                                            </form></div>\
                                            </html>";

void Web_fini()
{}

void handleUpdate(AsyncWebServerRequest* request)
{
    char* html = "<form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
    request->send(200, "text/html", html);
}

void handleUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final)
{
    if (!index)
        request->_tempFile = SPIFFS.open("/" + filename, "w");
    if (len)
        // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
    if (final)
    {
        request->_tempFile.close();
        request->redirect("/");
        if (filename == "config.json") {
            delay(200);
            ESP.restart();
	}
    }
}

void handleDoUpdate(AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final)
{
    String msg;

    if (!index)
    {
        msg = "updating firmware";

       //SPIFFS.format();

        // if filename includes spiffs, update the spiffs partition
        int cmd = (filename.indexOf("spiffs") > -1) ? U_PART : U_FLASH;

        if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd))
        {

            Update.printError(Serial);
        }
    }

    if (Update.write(data, len) != len)
    {
        Update.printError(Serial);
    }

    if (final)
    {

        Serial.printf("update %p/%d/%d\n", data, len, final);
        AsyncWebServerResponse* response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
        response->addHeader("Refresh", "20");
        response->addHeader("Location", "/");
        request->send(response);
        if (!Update.end(true))
            Update.printError(Serial);
        else
        {
            delay(1000);
            ESP.restart();
        }
    }
}

void Web_start()
{
    wserver.begin();
}

void Web_stop()
{
    wserver.end();
}

void Web_setup()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    if (!SPIFFS.exists("/index.html"))
    {
        wserver.on("/", HTTP_GET, [upload_html](AsyncWebServerRequest* request){
            request->send(200, "text/html", upload_html);
        });

        wserver.on("/doUpload", HTTP_POST, [](AsyncWebServerRequest* request) {}, handleUpload);

        Web_start();
        return;
    }

    File file = SPIFFS.open("/index.html", "r");
    if (!file)
    {
        Serial.println("An Error has occurred while opening index.html");
        return;
    }

    size_t filesize   = file.size();
    char*  index_html = (char *) malloc(filesize + 1);

    file.read((uint8_t *)index_html, filesize);
    index_html[filesize] = '\0';

    size_t size = 8700;
    char*  offset;
    char*  Settings_temp = (char *) malloc(size);

    if (Settings_temp == NULL)
        return;

    offset = Settings_temp;

    snprintf(offset, size, index_html,
             version.c_str(),
             wifi_ssid[0].c_str(), "hidepass",
             wifi_ssid[1].c_str(), "hidepass",
             wifi_ssid[2].c_str(), "hidepass",
             wifi_ssid[3].c_str(), "hidepass",
             wifi_ssid[4].c_str(), "hidepass",
             serverName.c_str(), serverPort,
             serverPath.c_str(), ntpServer.c_str(),
             gmtOffset_hour, daylightOffset_hour,
             caption.c_str(), timerInterval,
             start_upload, end_upload
             );

    size_t len  = strlen(offset);
    String html = String(offset);

    wserver.on("/", HTTP_GET, [html](AsyncWebServerRequest* request){
        request->send(200, "text/html", html);
    });


    // Route to load style.css file
    wserver.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* request){
        request->send(SPIFFS, "/style.css", "text/css");
    });

    wserver.on("/update", HTTP_GET, [](AsyncWebServerRequest* request){
        handleUpdate(request);
        request->redirect("/");
    });

    wserver.on("/doUpdate", HTTP_POST,
               [](AsyncWebServerRequest* request) {},
               [](AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data,
                  size_t len, bool final) {
        handleDoUpdate(request, filename, index, data, len, final);
    });

    wserver.on("/upload", HTTP_GET, [upload_html](AsyncWebServerRequest* request){
        request->send(200, "text/html", upload_html);
    });

    wserver.on("/reset_all", HTTP_GET, [](AsyncWebServerRequest* request){
        request->redirect("/");
        SPIFFS.format();
        delay(200);
        ESP.restart();
    });

    wserver.on("/reboot", HTTP_GET, [](AsyncWebServerRequest* request){
        request->redirect("/");
        ESP.restart();
    });

    wserver.on("/doUpload", HTTP_POST, [](AsyncWebServerRequest* request) {}, handleUpload);


    // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
    wserver.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
	int n_ssid = 0, n_pass = 0;
	int params = request->params();
	for(int i = 0; i < params; i++) {
	  AsyncWebParameter *p = request->getParam(i);
	  if (p->name() == String("wifi_ssid") &&
	      p->value() != String("xxxxxxx"))
	    wifi_ssid[n_ssid++] = p->value();
	  if (p->name() == String("wifi_password") &&
	      p->value() != String("hidepass"))
	    wifi_pass[n_pass++] = p->value();
	  if (p->name() == String("serverName"))
	    serverName = p->value();
	  if (p->name() == String("serverPort"))
	    serverPort = p->value().toInt();
	  if (p->name() == String("serverPath"))
	    serverPath = p->value();
	  if (p->name() == String("ntpServer"))
	    ntpServer = p->value();
	  if (p->name() == String("gmtOffset_hour"))
	    gmtOffset_hour = p->value().toInt();
	  if (p->name() == String("daylightOffset_hour"))
	    daylightOffset_hour = p->value().toInt();
	  if (p->name() == String("caption"))
	    caption = p->value();
	  if (p->name() == String("timerInterval"))
	    timerInterval = p->value().toInt();
	  if (p->name() == String("start_upload"))
	    start_upload = p->value().toInt();
	  if (p->name() == String("end_upload"))
	    end_upload = p->value().toInt();
	}

        request->redirect("/");

        save_config();
        delay(200);
        ESP.restart();
    });

    free(Settings_temp);
    free(index_html);

    // Start server
    Web_start();
}

void Web_loop(void)
{
}
