#include "mainserver.h"
#include <WiFi.h>
#include "task_check_info.h"

bool isAPMode = true;
unsigned long connect_start_ms = 0;
bool connecting = false;

void startAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), password.c_str());
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  isAPMode = true;
  connecting = false;
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  if (wifi_password.isEmpty()) {
    WiFi.begin(wifi_ssid.c_str());
  } else {
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  }
  Serial.print("Connecting to: ");
  Serial.print(wifi_ssid.c_str());
  Serial.print(" Password: ");
  Serial.println(wifi_password.c_str());
}

// ========== Main task ==========
void main_server_task(void *pvParameters) {
  // Always start AP so users can always find the device
  startAP();

  // If saved WiFi config exists → switch to AP+STA to connect to internet in parallel
  if (!WIFI_SSID.isEmpty()) {
    Serial.println("Found saved WiFi config. Switching to AP+STA mode...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid.c_str(), password.c_str());
    if (WIFI_PASS.isEmpty()) {
      WiFi.begin(WIFI_SSID.c_str());
    } else {
      WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }
    connecting = true;
    connect_start_ms = millis();
  }

  while (1) {
    if (connecting) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("STA IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("AP  IP address: ");
        Serial.println(WiFi.softAPIP());
        isWifiConnected = true;
        xSemaphoreGive(xBinarySemaphoreInternet);
        connecting = false;
      } else if (millis() - connect_start_ms > 15000) {
        Serial.println("WiFi STA connect failed! AP still running.");
        connecting = false;
        isWifiConnected = false;
      }
    }
    vTaskDelay(20);
  }
}