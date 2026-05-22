#include "task_toogle_boot.h"
#include "global.h"
#include "task_check_info.h"

#define BOOT_PIN 0
extern bool isAPMode;
extern void startAP();
extern void Webserver_sendata(String data);

void Task_Toogle_BOOT(void *pvParameters)
{
    pinMode(BOOT_PIN, INPUT_PULLUP);

    unsigned long buttonPressStartTime = 0;
    bool longPressHandled = false;

    while (true)
    {
        if (digitalRead(BOOT_PIN) == LOW)
        {
            if (buttonPressStartTime == 0)
            {
                buttonPressStartTime = millis();
                longPressHandled = false;
            }
            else if (!longPressHandled && millis() - buttonPressStartTime > 2000)
            {
                // Long press (>2s): factory reset
                longPressHandled = true;
                Serial.println("🔴 BOOT held >2s → Factory reset...");
                ws.textAll("{\"event\":\"factory_reset\",\"msg\":\"Config cleared. Restarting...\"}");
                vTaskDelay(300 / portTICK_PERIOD_MS); // allow WS message to flush
                Delete_info_File(); // deletes file then calls ESP.restart()
            }
        }
        else
        {
            if (buttonPressStartTime > 0 && !longPressHandled)
            {
                unsigned long held = millis() - buttonPressStartTime;
                if (held >= 50 && held < 2000)
                {
                    // Short press: switch to AP mode
                    Serial.println("🔵 BOOT short press → AP mode");
                    if (!isAPMode)
                    {
                        startAP();
                    }
                }
            }
            buttonPressStartTime = 0;
            longPressHandled = false;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}