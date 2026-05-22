#include "global.h"
float glob_temperature = 0;
float glob_humidity = 0;
bool humidity_red_zone = false;
bool temperature_red_zone = false;
bool neo_enabled = true;
bool blinky_enabled = true;

bool fan_auto_mode = true;
bool fan_state = false;
bool force_mqtt_telemetry = false; // Mặc định không bắt buộc publish liên tục
float glob_inference_result = -1.0; // Chưa có kết quả inference

String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

String ssid = "ESP32 - 121 Funny";
String password = "123456781";
String wifi_ssid = "";
String wifi_password = "";
boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

// Semaphores cho LED tasks (khởi tạo trong global.cpp, được pass qua main.cpp)
SemaphoreHandle_t ledSemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t neoSemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t dataMutex    = xSemaphoreCreateMutex();