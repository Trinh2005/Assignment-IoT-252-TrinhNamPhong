#include "neo_blinky.h"
#include "global.h"

void neo_blinky(void *pvParameters){
<<<<<<< Updated upstream
    SystemData_t *sysData = (SystemData_t *)pvParameters;
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    strip.show();

    uint32_t neoColor = strip.Color(0, 255, 0); 
    uint32_t blinkDelay = 0;
    bool isLedOn = true;

    while (1) {
        // Block task until the Sensor Task notifies a humidity change
        if (xSemaphoreTake(sysData->neoSemaphore, portMAX_DELAY)) {
            float humi;
            if (xSemaphoreTake(sysData->dataMutex, portMAX_DELAY)) {
                humi = sysData->humidity;
                xSemaphoreGive(sysData->dataMutex);
            }

            // TASK 2: Mapping humidity levels to 3 colors
            if (humi < 40.0) {
                // Level 1: Dry -> Orange
                neoColor = strip.Color(255, 100, 0);
                blinkDelay = 200; 
            } else if (humi >= 40.0 && humi <= 70.0) {
                // Level 2: Optimal -> Green
                neoColor = strip.Color(0, 255, 0);
                blinkDelay = 0;   
            } else {
                // Level 3: Humid -> Blue
                neoColor = strip.Color(0, 0, 255);
                blinkDelay = 1000;  
            }
            if (blinkDelay == 0) {
                strip.setPixelColor(0, neoColor);
                strip.show();
                vTaskDelay(pdMS_TO_TICKS(100)); 
            } else {
                if (isLedOn) {
                    strip.setPixelColor(0, 0); 
                } else {
                    strip.setPixelColor(0, neoColor); 
                }
                strip.show();
                isLedOn = !isLedOn;
                vTaskDelay(pdMS_TO_TICKS(blinkDelay));
            }
        }
=======

  Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
  strip.begin();

  // Tắt đèn khi mới khởi động
  strip.clear();
  strip.show();

  // Cache trạng thái hiện tại: 0=invalid, 1=low, 2=normal, 3=high
  int humidZone = 0;

  while(1) {
    if (!neo_enabled) {
      strip.clear();
      strip.show();
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
>>>>>>> Stashed changes
    }

    // Non-blocking semaphore take (timeout = 0).
    // Nếu Sensor Task đã give semaphore -> cập nhật vùng độ ẩm.
    if (xSemaphoreTake(neoSemaphore, 0) == pdTRUE) {
      float humidity = 0;

      // Đọc độ ẩm có bảo vệ bằng Mutex
      if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        humidity = glob_humidity;
        xSemaphoreGive(dataMutex);
      }

      if (humidity < 0 || isnan(humidity)) {
        humidZone = 0; // Dữ liệu không hợp lệ
      } else if (humidity < 35.0) {
        humidZone = 1; // Thấp: chớp vàng từ từ
      } else if (humidity <= 55.0) {
        humidZone = 2; // Bình thường: xanh lá cố định
      } else {
        humidZone = 3; // Cao: đỏ nhấp nháy
      }
    }

    // Thực thi hiệu ứng theo vùng hiện tại
    if (humidZone == 0) {
      strip.clear();
      strip.show();
      vTaskDelay(pdMS_TO_TICKS(500));
    } else if (humidZone == 1) {
      // Chớp vàng từ từ
      for(int i = 0; i <= 155; i += 5) {
        strip.setPixelColor(0, strip.Color(i, i, 0));
        strip.show();
        vTaskDelay(pdMS_TO_TICKS(10));
      }
      for(int i = 155; i >= 0; i -= 5) {
        strip.setPixelColor(0, strip.Color(i, i, 0));
        strip.show();
        vTaskDelay(pdMS_TO_TICKS(10));
      }
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
      vTaskDelay(pdMS_TO_TICKS(200));
    } else if (humidZone == 2) {
      // Màu xanh lá cố định
      strip.setPixelColor(0, strip.Color(0, 150, 0));
      strip.show();
      vTaskDelay(pdMS_TO_TICKS(500));
    } else {
      // Màu đỏ bật tắt liên tục
      strip.setPixelColor(0, strip.Color(150, 0, 0));
      strip.show();
      vTaskDelay(pdMS_TO_TICKS(200));
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
      vTaskDelay(pdMS_TO_TICKS(200));
    }
  }
}
