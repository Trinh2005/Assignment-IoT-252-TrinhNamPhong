
#ifndef __TASK_WEBSERVER_H__
#define __TASK_WEBSERVER_H__

#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include <task_handler.h>
#include "global.h"          // glob_temperature, glob_humidity, neo_enabled, ...
#include "task_check_info.h" // Save_info_File()

extern AsyncWebSocket ws;

void Webserver_stop();
void Webserver_reconnect();
void Webserver_sendata(String data);

#endif