#include "coreiot.h"

// ----------- CONFIGURE THESE! -----------
const char* coreIOT_Server = "app.coreiot.io";
const char* coreIOT_Token = "mioermbgdwqftctiyr8c";   // Device Access Token
const int   mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);


// Fixed client ID - do NOT use random ID, ThingsBoard treats each random ID
// as a NEW client and creates new subscription slots, never reusing the old ones.
static const char* MQTT_CLIENT_ID = "YOLO_UNO_121";

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Use token from saved config, fall back to hardcoded default
    String useToken = CORE_IOT_TOKEN.isEmpty() ? String(coreIOT_Token) : CORE_IOT_TOKEN;

    if (client.connect(MQTT_CLIENT_ID, useToken.c_str(), NULL)) {
      Serial.println("connected to CoreIOT Server!");

      // Unsubscribe first to clear any stale server-side subscription slots
      // This prevents "[TB] Too many server-side RPC subscriptions" accumulation
      client.unsubscribe("v1/devices/me/rpc/request/+");
      delay(50); // Brief pause to let unsubscribe propagate

      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      vTaskDelay(pdMS_TO_TICKS(5000));
    }
  }
}



void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* method = doc["method"];
  // App.coreiot (ThingsBoard) switches truyền kiểu bool: {"method":"setNeoLED","params":true}
  bool params = doc["params"] | false; 

  if (strcmp(method, "setNeoLED") == 0) {
    neo_enabled = params;
    Serial.printf("RPC setNeoLED: %s\n", params ? "ON" : "OFF");
  } 
  else if (strcmp(method, "setBlinkyLED") == 0) {
    blinky_enabled = params;
    Serial.printf("RPC setBlinkyLED: %s\n", params ? "ON" : "OFF");
  } 
  else if (strcmp(method, "setFanAuto") == 0) {
    fan_auto_mode = params;
    if (params) {
      // Vào AUTO: reset quạt về TẮT, để TinyML tự quyết định lại từ đầu
      fan_state = false;
      digitalWrite(FAN_PIN, LOW);
      Serial.println("RPC setFanAuto: AUTO (Fan reset OFF, TinyML will decide)");
    } else {
      // Vào MANUAL: giữ nguyên trạng thái quạt hiện tại, user tự điều khiển
      Serial.printf("RPC setFanAuto: MANUAL (Fan kept %s)\n", fan_state ? "ON" : "OFF");
    }
  } 
  else if (strcmp(method, "setFanState") == 0) {
    if (!fan_auto_mode) {
      fan_state = params;
      digitalWrite(FAN_PIN, fan_state ? HIGH : LOW);
      Serial.printf("RPC setFanState (Manual): %s\n", params ? "ON" : "OFF");
    } else {
      // AUTO mode: từ chối lệnh, gửi response báo trạng thái thực để Dashboard đồng bộ lại
      Serial.println("RPC setFanState: Rejected (Auto mode active)");
    }
  } 
  else {
    Serial.print("Unknown method: ");
    Serial.println(method);
    return;
  }

  // Gửi RPC response để Dashboard không bị treo timeout
  String topicStr = String(topic);
  int lastSlash = topicStr.lastIndexOf("/");
  if (lastSlash > 0) {
    String reqId = topicStr.substring(lastSlash + 1);
    String respTopic = "v1/devices/me/rpc/response/" + reqId;
    client.publish(respTopic.c_str(), "{\"status\":\"ok\"}");
  }

  // Đồng bộ Attributes ngay lập tức để tất cả Widget trên Dashboard hiển thị đúng
  String attrPayload = "{";
  attrPayload += "\"neo\":" + String(neo_enabled ? "true" : "false") + ",";
  attrPayload += "\"blinky\":" + String(blinky_enabled ? "true" : "false") + ",";
  attrPayload += "\"fanAuto\":" + String(fan_auto_mode ? "true" : "false") + ",";
  attrPayload += "\"fanOn\":" + String(fan_state ? "true" : "false");
  attrPayload += "}";
  client.publish("v1/devices/me/attributes", attrPayload.c_str());

  // Yêu cầu publish telemetry ngay lập tức
  force_mqtt_telemetry = true;
}


void setup_coreiot() {
  // Đợi cho đến khi WiFi kết nối thành công (semaphore từ mainserver)
  while (1) {
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
      break;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println(" WiFi Connected! Setting up MQTT...");

  static char mqttServer[128];
  if (CORE_IOT_SERVER.isEmpty()) {
    strncpy(mqttServer, coreIOT_Server, sizeof(mqttServer) - 1);
  } else {
    strncpy(mqttServer, CORE_IOT_SERVER.c_str(), sizeof(mqttServer) - 1);
  }
  mqttServer[sizeof(mqttServer) - 1] = '\0';

  int usePort = CORE_IOT_PORT.isEmpty() ? mqttPort : CORE_IOT_PORT.toInt();

  Serial.print("MQTT Connect to: ");
  Serial.print(mqttServer);
  Serial.print(":");
  Serial.println(usePort);

  client.setServer(mqttServer, usePort);
  client.setCallback(callback);
  client.setKeepAlive(120);    // 120s keepalive - default 15s is too short, causes reconnect loop
  client.setSocketTimeout(10); // 10s socket timeout
}

void coreiot_task(void *pvParameters){

    setup_coreiot();

    while(1){

        if (!client.connected()) {
            reconnect();
        }

        String payload = "{";
        payload += "\"temperature\":" + String(glob_temperature) + ",";
        payload += "\"humidity\":" + String(glob_humidity) + ",";
        payload += "\"neo\":" + String(neo_enabled ? "true" : "false") + ",";
        payload += "\"blinky\":" + String(blinky_enabled ? "true" : "false") + ",";
        payload += "\"fanAuto\":" + String(fan_auto_mode ? "true" : "false") + ",";
        payload += "\"fanOn\":" + String(fan_state ? "true" : "false") + ",";
        payload += "\"inference\":" + String(glob_inference_result, 3);
        payload += "}";
        
        client.publish("v1/devices/me/telemetry", payload.c_str());
        // Bắn luôn sang Attributes để các Widget Switch RPC trên Dashboard bắt được State dễ dàng
        client.publish("v1/devices/me/attributes", payload.c_str());
        
        Serial.println("Published telemetry & attributes: " + payload);
        
        // Thay vì ngủ 6 giây làm treo việc nhận RPC và publish, 
        // ta chia nhỏ vòng lặp và xử lý loop() liên tục siêu tốc (20ms/vòng).
        for (int i = 0; i < 300; i++) {
            client.loop(); // Nhận tín hiệu RPC siêu tốc
            if (force_mqtt_telemetry) {
                force_mqtt_telemetry = false;
                break; // Thoát vòng chờ để publish telemetry ngay
            }
            vTaskDelay(pdMS_TO_TICKS(20)); // Nghỉ 20ms để tương tác CoreIoT tức thì
        }
    }
}