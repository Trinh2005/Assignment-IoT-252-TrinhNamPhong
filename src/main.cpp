#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "mainserver.h"
#include "tinyml.h"
#include "coreiot.h"

// include task
#include "task_check_info.h"
// task_core_iot.h removed — ThingsBoard SDK in task_core_iot.cpp was creating a
// second MQTT connection that conflicts with coreiot_task (PubSubClient).
// coreiot_task is the correct implementation with full RPC device control.
#include "task_toogle_boot.h"
#include "task_webserver.h"
#include "task_wifi.h"


void setup() {
  Serial.begin(115200);
  check_info_File(0);

<<<<<<< Updated upstream
  // 1. Dynamically allocate the shared System Data structure in heap memory
  SystemData_t *sharedData = (SystemData_t *)malloc(sizeof(SystemData_t));
  sharedData->temperature = 0.0f;
  sharedData->humidity = 0.0f;

  // 2. Initialize RTOS Sync Primitives
  sharedData->dataMutex    = xSemaphoreCreateMutex();
  sharedData->ledSemaphore = xSemaphoreCreateBinary();
  sharedData->neoSemaphore = xSemaphoreCreateBinary();
  sharedData->lcdSemaphore = xSemaphoreCreateBinary();

  xTaskCreate(led_blinky, "Task LED Blink", 2048, (void *)sharedData, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, (void *)sharedData, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 4096, (void *)sharedData, 2, NULL);
  // xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
  // xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
  // xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
=======
  xTaskCreate(led_blinky,         "Task LED Blink",        2560, NULL, 2, NULL);
  xTaskCreate(neo_blinky,         "Task NEO Blink",        2560, NULL, 2, NULL);
  xTaskCreate(temp_humi_monitor,  "Task TEMP HUMI Monitor",4096, NULL, 2, NULL);
  xTaskCreate(main_server_task,   "Task Main Server",      8192, NULL, 2, NULL);
  xTaskCreate(tiny_ml_task,       "Tiny ML Task",          4096, NULL, 2, NULL);
  xTaskCreate(coreiot_task,       "CoreIOT Task",          4096, NULL, 2, NULL);
  xTaskCreate(Task_Toogle_BOOT,   "Task_Toogle_BOOT",      4096, NULL, 2, NULL);
>>>>>>> Stashed changes
}

void loop() {
  // WebServer always runs — needed for both AP config page and STA dashboard
  Webserver_reconnect();

  // WiFi check for AP+STA mode (does not stop webserver on failure)
  if (check_info_File(1)) {
    Wifi_reconnect();
    // CoreIOT is managed entirely by coreiot_task (FreeRTOS) — do NOT call here
  }
}