#include "MyWiFi.h"

#define WIFI_TIMEOUT_MS 20000

static char TAG[] = "MyWiFi";

void WiFiKeepAliveTask(void *pvParameters) {
  while (1) {
    if (WiFi.status() == WL_CONNECTED) {
      ESP_LOGI(TAG, "WiFi still connected");
      delay(30000);
      continue;
    }
    ESP_LOGI(TAG, "WiFi Connecting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
    }

    if (WiFi.status() != WL_CONNECTED) {
      ESP_LOGE(TAG, "FAILED");
      delay(20000);
      continue;
    }

    ESP_LOGI(TAG, "Connected");
  }
  vTaskDelete(NULL);
}

int myWiFibegin() {
  WiFi.mode(WIFI_STA);
  if (WiFi.begin(WIFI_SSID, WIFI_PASS) != WL_DISCONNECTED) {
    ESP_LOGE(TAG, "Not Status WL_DISCONNECTED");
    ESP.restart();
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  ESP_LOGI(TAG, "WL_CONNECTED");

  return 0;
}

int watchWiFiStatus() {
  static int rtn_code = 0;

  switch (WiFi.status()) {
    case WL_IDLE_STATUS:
      ESP_LOGI(TAG, "WL_IDLE_STATUS");
      myWiFibegin();
      rtn_code = 0;
      break;
    case WL_SCAN_COMPLETED:
      ESP_LOGI(TAG, "WL_SCAN_COMPLETED");
      rtn_code = 0;
      break;
    case WL_CONNECTED:
      ESP_LOGI(TAG, "WL_CONNECTED");
      rtn_code = 0;
      break;
    case WL_CONNECT_FAILED:
      ESP_LOGE(TAG, "WL_CONNECT_FAILED");
      WiFi.disconnect();
      delay(500);
      myWiFibegin();
      rtn_code = 0;
      break;
    case WL_CONNECTION_LOST:
      ESP_LOGW(TAG, "WL_CONNECTION_LOST");
      WiFi.reconnect();
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
      }
      rtn_code = 0;
      break;
    case WL_DISCONNECTED:
      ESP_LOGW(TAG, "WL_DISCONNECTED");
      myWiFibegin();
      rtn_code = 0;
      break;
    default:
      ESP_LOGE(TAG, "watchWiFiStatus switched default %d", (int)WiFi.status());
      WiFi.disconnect();
      delay(500);
      myWiFibegin();
      rtn_code = 0;
      break;
  }

  return rtn_code;
}
