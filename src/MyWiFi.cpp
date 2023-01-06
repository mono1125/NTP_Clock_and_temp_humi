#include "MyWiFi.h"

static char TAG[] = "MyWiFi";

static void myWiFibegin() {
  WiFi.mode(WIFI_STA);
  if (WiFi.begin(WIFI_SSID, WIFI_PASS) != WL_DISCONNECTED) {
    ESP_LOGE(TAG, "Not Status WL_DISCONNECTED");
    ESP.restart();
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  ESP_LOGI(TAG, "WL_CONNECTED");
}

int watchWiFiStatus() {
  static int rtn_code = 0;

  switch (WiFi.status()) {
    case WL_CONNECTED:
      rtn_code = 0;
      break;
    case WL_CONNECTION_LOST:
      WiFi.reconnect();
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
      }
      rtn_code = 0;
      break;
    case WL_CONNECT_FAILED:
      WiFi.disconnect();
      delay(500);
      myWiFibegin();
      rtn_code = 0;
      break;
    case WL_DISCONNECTED:
      myWiFibegin();
      rtn_code = 0;
      break;
    default:
      ESP_LOGE(TAG, "watchWiFiStatus switched default");
      WiFi.disconnect();
      delay(500);
      myWiFibegin();
      rtn_code = 0;
      break;
  }

  return rtn_code;
}
