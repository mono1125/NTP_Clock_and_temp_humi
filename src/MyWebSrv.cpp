#include "MyWebSrv.h"

static char TAG[] = "MyWebSrv";

static AsyncWebServer server(HTTP_PORT);

void onRequest(AsyncWebServerRequest *request){
  // Handle Unknown Request
  request->send(404);
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  if(!request->authenticate(HTTP_USER, HTTP_PASS)){
    return request->requestAuthentication();
  }
  if ((request->url() == "/api/config") && (request->method() == HTTP_POST)){
    /*
      Sample Post Data: {"message": "post-data"}
    */
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, (const char*)data);
    if(doc.containsKey("message")){
      const char* buf = doc["message"];
      ESP_LOGI(TAG, "Deserialized Json [message]: %s", buf);
    }

    request->send(200, "application/json", "{\"message\":\"post method response\"}");
  }
}

void myWebSrv() {
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(HTTP_USER, HTTP_PASS)){
      return request->requestAuthentication();
    }
    request->send(200, "text/plain", String(esp_get_free_heap_size()));
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(HTTP_USER, HTTP_PASS)){
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

  server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request){
    // TODO: 現在の設定値を返す
    if(!request->authenticate(HTTP_USER, HTTP_PASS)){
      return request->requestAuthentication();
    }
    request->send(200, "application/json", "{\"message\":\"sample parameter\"}");
  });

  // Catch-All Handler
  server.onRequestBody(onBody);
  server.onNotFound(onRequest);

  server.begin();
}
