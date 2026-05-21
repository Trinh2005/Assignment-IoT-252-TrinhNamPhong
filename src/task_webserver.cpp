#include "task_webserver.h"

AsyncWebServer asyncServer(80);
AsyncWebSocket ws("/ws");

bool webserver_isrunning = false;

void Webserver_sendata(String data)
{
    if (ws.count() > 0)
    {
        ws.textAll(data);
        Serial.println("Sent WS: " + data);
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("WS client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());

        // Push current state immediately to new client so dashboard shows data without waiting
        String resp = "{\"page\":\"telemetry\"";
        resp += ",\"temp\":" + String(glob_temperature, 1);
        resp += ",\"hum\":" + String(glob_humidity, 1);
        resp += ",\"ml\":" + String(glob_inference_result, 3);
        resp += ",\"neo\":\"" + String(neo_enabled ? "ON" : "OFF") + "\"";
        resp += ",\"blinky\":\"" + String(blinky_enabled ? "ON" : "OFF") + "\"";
        resp += ",\"fanAuto\":\"" + String(fan_auto_mode ? "AUTO" : "MANUAL") + "\"";
        resp += ",\"fanState\":\"" + String(fan_state ? "ON" : "OFF") + "\"";
        resp += "}";
        client->text(resp);
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("WS client #%u disconnected\n", client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->opcode == WS_TEXT)
        {
            String message;
            message += String((char *)data).substring(0, len);
            handleWebSocketMessage(message);
        }
    }
}

void connnectWSV()
{
    ws.onEvent(onEvent);
    asyncServer.addHandler(&ws);

    // --- AP Captive Portal route: handles form submission from test.html ---
    // test.html POSTs via: /connect?ssid=...&pass=...&token=...&server=...&port=...
    asyncServer.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request) {
        String ssid_v   = request->hasParam("ssid")   ? request->getParam("ssid")->value()   : "";
        String pass_v   = request->hasParam("pass")   ? request->getParam("pass")->value()   : "";
        String token_v  = request->hasParam("token")  ? request->getParam("token")->value()  : "";
        String server_v = request->hasParam("server") ? request->getParam("server")->value() : "";
        String port_v   = request->hasParam("port")   ? request->getParam("port")->value()   : "";

        if (ssid_v.isEmpty() || token_v.isEmpty() || server_v.isEmpty() || port_v.isEmpty()) {
            request->send(400, "text/plain; charset=utf-8", "Thieu thong so bat buoc!");
            return;
        }

        Serial.println("AP /connect received. SSID=" + ssid_v);
        // Save_info_File calls ESP.restart() internally, so response below is best-effort
        request->send(200, "text/plain; charset=utf-8", "Thanh cong! Thiet bi dang khoi dong lai...");
        delay(100);
        Save_info_File(ssid_v, pass_v, token_v, server_v, port_v);
    });

    // --- REST endpoint: return current telemetry/state as JSON (for page-load sync) ---
    asyncServer.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String resp = "{\"page\":\"telemetry\"";
        resp += ",\"temp\":" + String(glob_temperature, 1);
        resp += ",\"hum\":" + String(glob_humidity, 1);
        resp += ",\"ml\":" + String(glob_inference_result, 3);
        resp += ",\"neo\":\"" + String(neo_enabled ? "ON" : "OFF") + "\"";
        resp += ",\"blinky\":\"" + String(blinky_enabled ? "ON" : "OFF") + "\"";
        resp += ",\"fanAuto\":\"" + String(fan_auto_mode ? "AUTO" : "MANUAL") + "\"";
        resp += ",\"fanState\":\"" + String(fan_state ? "ON" : "OFF") + "\"";
        resp += "}";
        request->send(200, "application/json", resp);
    });

    // --- Serve LittleFS static files (index.html, styles.css, script.js) ---
    asyncServer.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    asyncServer.begin();
    ElegantOTA.begin(&asyncServer);
    webserver_isrunning = true;
    Serial.println("WebServer started.");
}

void Webserver_stop()
{
    ws.closeAll();
    asyncServer.end();
    webserver_isrunning = false;
}

void Webserver_reconnect()
{
    if (!webserver_isrunning)
    {
        connnectWSV();
    }
    ElegantOTA.loop();
    ws.cleanupClients(); // Prevent WS memory leak from stale clients
}