#include "temp_humi_monitor.h"
#include <Wire.h>

extern void Webserver_sendata(String data);
DHT20 dht20;
LiquidCrystal_I2C lcd(33, 16, 2);

void temp_humi_monitor(void *pvParameters) {

<<<<<<< Updated upstream
void temp_humi_monitor(void *pvParameters){
    SystemData_t *sysData = (SystemData_t *)pvParameters;
    Wire.begin(11, 12);
    Serial.begin(115200);
    dht20.begin();
    lcd.begin(); 
    lcd.backlight();

    float lastTemp = -100.0;
    float lastHumi = -100.0;
    while (1){
        /* code */
        
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();

        bool dht_failed = isnan(temperature) || isnan(humidity);
        bool temperature_red_zone = (temperature > 30.0);
        bool humidity_blue_zone = (humidity > 70.0);
        bool humidity_yellow_zone = (humidity < 40.0);
        

        // Check if any reads failed and exit early
        if (dht_failed) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity =  -1;
            xSemaphoreGive(sysData->lcdSemaphore);
            //return;
        } else {
            if (xSemaphoreTake(sysData->dataMutex, portMAX_DELAY)) {
                sysData->temperature = temperature;
                sysData->humidity = humidity;
                xSemaphoreGive(sysData->dataMutex);
            }
            if (abs(temperature - lastTemp) > 0.2) {
                xSemaphoreGive(sysData->ledSemaphore);
                lastTemp = temperature;
            }
            if (abs(humidity - lastHumi) > 1.0) {
                xSemaphoreGive(sysData->neoSemaphore);
                lastHumi = humidity;
            }
            xSemaphoreGive(sysData->lcdSemaphore);
        }

        // //Update global variables for temperature and humidity
        // glob_temperature = temperature;
        // glob_humidity = humidity;

        // xSemaphoreGive(xHumiditySemaphore);

        //Print the results
        if (dht_failed) {
            Serial.println("Humidity: --%  Temperature: --°C");
        } else {
            Serial.print("Humidity: ");
            Serial.print(humidity);
            Serial.print("%  Temperature: ");
            Serial.print(temperature);
            Serial.println("°C");
        }

        
        if (xSemaphoreTake(sysData->lcdSemaphore, 0) == pdTRUE) {
            lcd.clear();
            static bool alternate_message = false;
            
            if (dht_failed) {
                lcd.setCursor(0, 0);
                lcd.print("DHT20 read error");
                lcd.setCursor(0, 1);
                lcd.print("Check sensor");
            } else if (humidity_blue_zone && temperature_red_zone) {
                alternate_message = !alternate_message;
                if (alternate_message) {
                    lcd.setCursor(0, 0);
                    lcd.print("HUMID TOO HIGH");
                } else {
                    lcd.setCursor(0, 0);
                    lcd.print("TEMP TOO HOT");
                }
            } else if (humidity_yellow_zone && temperature_red_zone) {
                alternate_message = !alternate_message;
                if (alternate_message) {
                    lcd.setCursor(0, 0);
                    lcd.print("HUMID TOO LOW");
                } else {
                    lcd.setCursor(0, 0);
                    lcd.print("TEMP TOO HOT");
                }
            } else if (humidity_blue_zone) {
                lcd.setCursor(0, 0);
                lcd.print("HUMID TOO HIGH");
                lcd.setCursor(0, 1);
                lcd.print("Hum:");
                lcd.print(humidity, 1);
                lcd.print("%");
            } else if (temperature_red_zone) {
                lcd.setCursor(0, 0);
                lcd.print("TEMP TOO HOT");
                lcd.setCursor(0, 1);
                lcd.print("Temp:");
                lcd.print(temperature, 1);
                lcd.print((char)223);
                lcd.print("C");
            } else if (humidity_yellow_zone) {
                lcd.setCursor(0, 0);
                lcd.print("HUMID TOO LOW");
                lcd.setCursor(0, 1);
                lcd.print("Hum:");
                lcd.print(humidity, 1);
                lcd.print("%");
            } else {
                lcd.setCursor(0, 0);
                lcd.print("Temp:");
                lcd.print(temperature, 1);
                lcd.print((char)223);
                lcd.print("C");
                lcd.setCursor(0, 1);
                lcd.print("Hum:");
                lcd.print(humidity, 1);
                lcd.print("%");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
=======
  Wire.begin(11, 12);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  Serial.begin(115200);
  dht20.begin();

  while (1) {
    dht20.read();
    float temperature = dht20.getTemperature();
    float humidity = dht20.getHumidity();

    bool dht_failed = isnan(temperature) || isnan(humidity);
    if (dht_failed) {
      Serial.println("Failed to read from DHT sensor!");
      temperature = humidity = -1;
>>>>>>> Stashed changes
    }

    // Ghi dữ liệu vào biến toàn cục có bảo vệ bằng Mutex
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      glob_temperature = temperature;
      glob_humidity = humidity;
      humidity_red_zone = (!dht_failed && humidity > 55.0);
      temperature_red_zone = (!dht_failed && temperature > 45.0);
      xSemaphoreGive(dataMutex);
    }
    bool humidity_yellow_zone = (!dht_failed && humidity < 35.0);

    // Push full telemetry (sensor + device state) to WebSocket clients
    String wsPayload = "{\"page\":\"telemetry\"";
    wsPayload += ",\"temp\":" + String(temperature, 1);
    wsPayload += ",\"hum\":" + String(humidity, 1);
    wsPayload += ",\"ml\":" + String(glob_inference_result, 3);
    wsPayload += ",\"neo\":\"" + String(neo_enabled ? "ON" : "OFF") + "\"";
    wsPayload += ",\"blinky\":\"" + String(blinky_enabled ? "ON" : "OFF") + "\"";
    wsPayload += ",\"fanAuto\":\"" + String(fan_auto_mode ? "AUTO" : "MANUAL") + "\"";
    wsPayload += ",\"fanState\":\"" + String(fan_state ? "ON" : "OFF") + "\"";
    wsPayload += "}";
    Webserver_sendata(wsPayload);

    // Thông báo cho LED tasks rằng có dữ liệu mới
    xSemaphoreGive(ledSemaphore);
    xSemaphoreGive(neoSemaphore);

    // Print the results on Serial
    if (dht_failed) {
      Serial.println("Humidity: --%  Temperature: --°C");
    } else {
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.print("%  Temperature: ");
      Serial.print(temperature);
      Serial.println("°C");
    }

    lcd.clear();
    static bool alternate_red_message = false;
    if (dht_failed) {
      lcd.setCursor(0, 0);
      lcd.print("DHT20 read error");
      lcd.setCursor(0, 1);
      lcd.print("Check sensor");
    } else if (humidity_red_zone && temperature_red_zone) {
      // Cả 2 đều đỏ — xen kẽ hiển thị
      alternate_red_message = !alternate_red_message;
      if (alternate_red_message) {
        lcd.setCursor(0, 0);
        lcd.print("HUMID TOO HIGH");
        lcd.setCursor(0, 1);
        lcd.print("Hum:");
        lcd.print(humidity, 1);
        lcd.print("%");
      } else {
        lcd.setCursor(0, 0);
        lcd.print("TEMP TOO HOT");
        lcd.setCursor(0, 1);
        lcd.print("Temp:");
        lcd.print(temperature, 1);
        lcd.print((char)223);
        lcd.print("C");
      }
    } else if (humidity_yellow_zone && temperature_red_zone) {
      // Humidity thấp + nhiệt độ cao — xen kẽ hiển thị
      alternate_red_message = !alternate_red_message;
      if (alternate_red_message) {
        lcd.setCursor(0, 0);
        lcd.print("HUMID TOO LOW");
        lcd.setCursor(0, 1);
        lcd.print("Hum:");
        lcd.print(humidity, 1);
        lcd.print("%");
      } else {
        lcd.setCursor(0, 0);
        lcd.print("TEMP TOO HOT");
        lcd.setCursor(0, 1);
        lcd.print("Temp:");
        lcd.print(temperature, 1);
        lcd.print((char)223);
        lcd.print("C");
      }
    } else if (humidity_red_zone) {
      lcd.setCursor(0, 0);
      lcd.print("HUMID TOO HIGH");
      lcd.setCursor(0, 1);
      lcd.print("Hum:");
      lcd.print(humidity, 1);
      lcd.print("%");
    } else if (temperature_red_zone) {
      lcd.setCursor(0, 0);
      lcd.print("TEMP TOO HOT");
      lcd.setCursor(0, 1);
      lcd.print("Temp:");
      lcd.print(temperature, 1);
      lcd.print((char)223);
      lcd.print("C");
    } else if (humidity_yellow_zone) {
      lcd.setCursor(0, 0);
      lcd.print("HUMID TOO LOW");
      lcd.setCursor(0, 1);
      lcd.print("Hum:");
      lcd.print(humidity, 1);
      lcd.print("%");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.print(temperature, 1);
      lcd.print((char)223);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("Hum:");
      lcd.print(humidity, 1);
      lcd.print("%");
    }

    vTaskDelay(2415 / portTICK_PERIOD_MS);
  }
}