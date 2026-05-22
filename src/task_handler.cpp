#include <task_handler.h>

void handleWebSocketMessage(String message)
{
    Serial.println(message);
    StaticJsonDocument<256> doc;

    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
        Serial.println("❌ Lỗi parse JSON!");
        return;
    }
    JsonObject value = doc["value"];
    if (doc["page"] == "device")
    {
        if (!value.containsKey("gpio") || !value.containsKey("status"))
        {
            Serial.println("⚠️ JSON thiếu thông tin gpio hoặc status");
            return;
        }

        int gpio = value["gpio"];
        String status = value["status"].as<String>();

        Serial.printf("⚙️ Điều khiển GPIO %d → %s\n", gpio, status.c_str());
        pinMode(gpio, OUTPUT);
        if (status.equalsIgnoreCase("ON"))
        {
            digitalWrite(gpio, HIGH);
            Serial.printf("🔆 GPIO %d ON\n", gpio);
        }
        else if (status.equalsIgnoreCase("OFF"))
        {
            digitalWrite(gpio, LOW);
            Serial.printf("💤 GPIO %d OFF\n", gpio);
        }
    }
    else if (doc["page"] == "control")
    {
        String device = doc["device"].as<String>();

        if (device == "neo")
        {
            neo_enabled = !neo_enabled;
            Serial.printf("Neo Pixel -> %s\n", neo_enabled ? "ON" : "OFF");
        }
        else if (device == "blinky")
        {
            blinky_enabled = !blinky_enabled;
            Serial.printf("Blinky LED -> %s\n", blinky_enabled ? "ON" : "OFF");
        }
        else if (device == "fanAuto")
        {
            fan_auto_mode = !fan_auto_mode;
            if (fan_auto_mode)
            {
                fan_state = false;
                digitalWrite(FAN_PIN, LOW);
                Serial.println("Fan Mode -> AUTO (TinyML controls fan)");
            }
            else
            {
                Serial.printf("Fan Mode -> MANUAL (kept %s)\n", fan_state ? "ON" : "OFF");
            }
        }
        else if (device == "fanState" && !fan_auto_mode)
        {
            fan_state = !fan_state;
            digitalWrite(FAN_PIN, fan_state ? HIGH : LOW);
            Serial.printf("Manual Fan -> %s\n", fan_state ? "ON" : "OFF");
        }

        force_mqtt_telemetry = true;

        // Echo updated telemetry to all WS clients so all UIs stay in sync
        String resp = "{\"page\":\"telemetry\"";
        resp += ",\"temp\":" + String(glob_temperature, 1);
        resp += ",\"hum\":" + String(glob_humidity, 1);
        resp += ",\"ml\":" + String(glob_inference_result, 3);
        resp += ",\"neo\":\"" + String(neo_enabled ? "ON" : "OFF") + "\"";
        resp += ",\"blinky\":\"" + String(blinky_enabled ? "ON" : "OFF") + "\"";
        resp += ",\"fanAuto\":\"" + String(fan_auto_mode ? "AUTO" : "MANUAL") + "\"";
        resp += ",\"fanState\":\"" + String(fan_state ? "ON" : "OFF") + "\"";
        resp += "}";
        ws.textAll(resp);
    }
    else if (doc["page"] == "setting")
    {
        String WIFI_SSID = doc["value"]["ssid"].as<String>();
        String WIFI_PASS = doc["value"]["password"].as<String>();
        String CORE_IOT_TOKEN = doc["value"]["token"].as<String>();
        String CORE_IOT_SERVER = doc["value"]["server"].as<String>();
        String CORE_IOT_PORT = doc["value"]["port"].as<String>();

        Serial.println("📥 Nhận cấu hình từ WebSocket:");
        Serial.println("SSID: " + WIFI_SSID);
        Serial.println("PASS: " + WIFI_PASS);
        Serial.println("TOKEN: " + CORE_IOT_TOKEN);
        Serial.println("SERVER: " + CORE_IOT_SERVER);
        Serial.println("PORT: " + CORE_IOT_PORT);

        // 👉 Gọi hàm lưu cấu hình
        Save_info_File(WIFI_SSID, WIFI_PASS, CORE_IOT_TOKEN, CORE_IOT_SERVER, CORE_IOT_PORT);

        // Phản hồi lại client (tùy chọn)
        String msg = "{\"status\":\"ok\",\"page\":\"setting_saved\"}";
        ws.textAll(msg);
    }
}
