#include "MyWebSrv.h"

static char TAG[] = "MyWebSrv";
/* 内部 */
static void printWebSrvInfo();
/* 内部 */

static AsyncWebServer server(HTTP_PORT);

static void onRequest(AsyncWebServerRequest *request) {
  // Handle Unknown Request
  request->send(404);
}

static void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if (!request->authenticate(HTTP_USER, HTTP_PASS)) {
    return request->requestAuthentication();
  }
  if ((request->url() == "/api/config") && (request->method() == HTTP_POST)) {
    ESP_LOGI(TAG, "Post Event /api/config");

    DynamicJsonDocument doc(2048);
    if (myDeserializeJson(doc, (char *)data) != 0) {
      return request->send(500, "application/json", "{\"message\":\"Deserialization Error\"}");
    }
    switch (checkParams(doc)) {
      case 0:
        break;
      case -1:
        return request->send(400, "application/json",
                             "{\"message\": \"Invalid paramater! Error: check name config params\"}");
        break;
      case -2:
        return request->send(400, "application/json",
                             "{\"message\": \"Invalid paramater! Error: check exists config params\"}");
        break;
      case -3:
        return request->send(400, "application/json",
                             "{\"message\": \"Invalid paramater! Error: check length config params\"}");
        break;
      case -4:
        return request->send(400, "application/json",
                             "{\"message\": \"Invalid paramater! Error: check range config params\"}");
        break;
      default:
        ESP_LOGE(TAG, "UNKNOWN ERROR");
        break;
    }

    if (writeJsonFile("/config.json", doc) != 0) {
      return request->send(500, "application/json", "{\"message\":\"Write JSON File Error\"}");
    }

    return request->send(201, "application/json", "{\"message\":\"Write config.json Success!\"}");
  }
}

void myWebSrv() {
  printWebSrvInfo();

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(HTTP_USER, HTTP_PASS)) {
      return request->requestAuthentication();
    }
    request->send(200, "text/plain", String(esp_get_free_heap_size()));
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(HTTP_USER, HTTP_PASS)) {
      return request->requestAuthentication();
    }
    request->send(200, "text/plain", "Hello!");
  });

  /* --- TODO ---
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(HTTP_USER, HTTP_PASS)){
      return request->requestAuthentication();
    }
    request->send(LittleFS, "www/index.html");
  });
  server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(HTTP_USER, HTTP_PASS)){
      return request->requestAuthentication();
    }
    request->send(LittleFS, "www/main.js");
  });
  server.on("/main.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "www/main.css");
  });
  --- TODO ---*/

  server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(HTTP_USER, HTTP_PASS)) {
      return request->requestAuthentication();
    }
    if (LittleFS.exists("/config.json")) {
      ESP_LOGD(TAG, "response: /config.json");
      return request->send(LittleFS, "/config.json");
    } else if (LittleFS.exists("/default_config.json")) {
      ESP_LOGD(TAG, "response: /default_config.json");
      return request->send(LittleFS, "/default_config.json");
    } else {
      ESP_LOGD(TAG, "response: Not exists Config Files");
      return request->send(500, "application/json", "{\"message\":\"Not exists Config Files\"}");
    }
  });

  /* For test */
  server.on("/api/new-config", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(HTTP_USER, HTTP_PASS)) {
      return request->requestAuthentication();
    }
    if (LittleFS.exists("/new_config.json")) {
      ESP_LOGD(TAG, "response: /new_config.json");
      return request->send(LittleFS, "/new_config.json");
    } else {
      ESP_LOGD(TAG, "response: Not exists Config Files");
      return request->send(500, "application/json", "{\"message\":\"Not exists Config Files\"}");
    }
  });
  /* For test */

  server.on("/api/thing", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(HTTP_USER, HTTP_PASS)) {
      return request->requestAuthentication();
    }
    static char thing_buf[50];
    // {"thing": "default thing (aws iot)"}
    sprintf(thing_buf, "{\"thing\":\"" THING_NAME "\"}");
    return request->send(200, "application/json", thing_buf);
  });

  server.on("/api/ap-list", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate(HTTP_USER, HTTP_PASS)) {
      return request->requestAuthentication();
    }
    static char ap_buf[2048];
    scanWiFiAP(ap_buf, sizeof(ap_buf));
    return request->send(200, "application/json", ap_buf);
  });

  // Catch-All Handler
  server.onRequestBody(onBody);
  server.onNotFound(onRequest);

  server.begin();
}

static void printWebSrvInfo() {
  ESP_LOGI(TAG, "--- Web Server Info ---");
  ESP_LOGI(TAG, "HTTP_PORT: %d", HTTP_PORT);
  ESP_LOGI(TAG, "HTTP_USER: %s", HTTP_USER);
  ESP_LOGI(TAG, "HTTP_PASS: %s", HTTP_PASS);
  ESP_LOGI(TAG, "--- Web Server Info ---");
}