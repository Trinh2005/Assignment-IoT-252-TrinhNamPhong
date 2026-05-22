#ifndef ___MAIN_SERVER__
#define ___MAIN_SERVER__
#include <Arduino.h>
#include <WiFi.h>
#include "global.h"

#define LED1_PIN 48
#define LED2_PIN 41
#define BOOT_PIN 0
#define FAN_PIN  2

extern bool isAPMode;

void startAP();
void connectToWiFi();
void main_server_task(void *pvParameters);

#endif
