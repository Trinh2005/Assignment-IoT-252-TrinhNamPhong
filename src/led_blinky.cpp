#include "led_blinky.h"

void led_blinky(void *pvParameters) {
<<<<<<< Updated upstream
    SystemData_t *sysData = (SystemData_t *)pvParameters;
    pinMode(LED_GPIO, OUTPUT);
    uint32_t blinkDelay = 1000; // Default

    while (1) {
        // Non-blocking semaphore take (Timeout = 0). 
        // If Sensor Task gave the semaphore, update the blink speed.
        if (xSemaphoreTake(sysData->ledSemaphore, 0) == pdTRUE) {
            float temp;
            if (xSemaphoreTake(sysData->dataMutex, portMAX_DELAY)) {
                temp = sysData->temperature;
                xSemaphoreGive(sysData->dataMutex);
            }

            // TASK 1: 3 different behaviors based on temperature conditions
            if (temp < 25.0) {
                blinkDelay = 2000; // Cold: Slow blink
            } else if (temp >= 25.0 && temp < 30.0) {
                blinkDelay = 500;  // Optimal: Fast blink
            } else {
                blinkDelay = 100;  // Hot: Very fast blink
            }
        }

        // Execute the blink behavior
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(blinkDelay));
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(blinkDelay));
    }
=======
  pinMode(LED_GPIO, OUTPUT);

  // Mặc định: giả sử bình thường (< 35°C)
  uint32_t onDelay  = 500;
  uint32_t offDelay = 0;   // 0 = không tắt (sáng liên tục trong khoảng delay)

  while (1) {
    if (!blinky_enabled) {
      digitalWrite(LED_GPIO, LOW);
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }

    // Non-blocking semaphore take (timeout = 0).
    // Nếu Sensor Task đã give semaphore -> cập nhật tốc độ nhấp nháy.
    if (xSemaphoreTake(ledSemaphore, 0) == pdTRUE) {
      float temp = 0;

      // Đọc nhiệt độ có bảo vệ bằng Mutex
      if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        temp = glob_temperature;
        xSemaphoreGive(dataMutex);
      }

      if (temp < 0 || isnan(temp)) {
        // Dữ liệu không hợp lệ: tắt đèn tạm thời
        onDelay  = 0;
        offDelay = 500;
      } else if (temp < 35.0) {
        // Bình thường: sáng liên tục (nhấp nháy chậm 500ms ON)
        onDelay  = 500;
        offDelay = 0;
      } else if (temp <= 45.0) {
        // Cảnh báo: chớp chậm 2 giây
        onDelay  = 1000;
        offDelay = 1000;
      } else {
        // Nguy hiểm: chớp nhanh 250ms
        onDelay  = 250;
        offDelay = 250;
      }
    }

    // Thực thi hành vi nhấp nháy hiện tại
    if (onDelay > 0) {
      digitalWrite(LED_GPIO, HIGH);
      vTaskDelay(pdMS_TO_TICKS(onDelay));
    }
    if (offDelay > 0) {
      digitalWrite(LED_GPIO, LOW);
      vTaskDelay(pdMS_TO_TICKS(offDelay));
    }
  }
>>>>>>> Stashed changes
}