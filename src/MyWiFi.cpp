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
    WiFi.reconnect();

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

int begin2STAForRUN(const Config *p) {
  WiFi.mode(WIFI_STA);
  if (p->useDhcp == 0) {
    IPAddress local_ip, gateway, subnet;
    IPAddress pub_dns(8, 8, 8, 8);  // Google Public DNS
    if ((isEmptyChar(p->localIPAddress) == 0) || (isEmptyChar(p->gatewayAddress) == 0) ||
        (isEmptyChar(p->subnetMask) == 0)) {
      ESP_LOGE(TAG, "IPアドレスが空文字");
      return -1;
    }
    if ((char2IPAddress(&local_ip, p->localIPAddress) != 0) || (char2IPAddress(&gateway, p->gatewayAddress) != 0) ||
        (char2IPAddress(&subnet, p->subnetMask) != 0)) {
      ESP_LOGE(TAG, "文字列からIPアドレスの変換に失敗");
      return -1;
    }
    if (!WiFi.config(local_ip, gateway, subnet, gateway, pub_dns)) {
      ESP_LOGE(TAG, "IPアドレスの設定に失敗");
      return -1;
    }
  }
  if (WiFi.begin(p->wifiSsid, p->wifiPass) != WL_DISCONNECTED) {
    ESP_LOGE(TAG, "Not Status WL_DISCONNECTED");
    ESP.restart();
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  ESP_LOGI(TAG, "WL_CONNECTED");

  return 0;
}

int begin2STAForCONFIG(const Config *p) {
  WiFi.mode(WIFI_STA);
  IPAddress local_ip, gateway, subnet;
  if ((isEmptyChar(p->gatewayAddress) == 0) || (isEmptyChar(p->subnetMask) == 0)) {
    ESP_LOGE(TAG, "IPアドレスが空文字");
    return -1;
  }
  if ((char2IPAddress(&local_ip, CONF_MODE_IP) != 0) || (char2IPAddress(&gateway, p->gatewayAddress) != 0) ||
      (char2IPAddress(&subnet, p->subnetMask) != 0)) {
    ESP_LOGE(TAG, "文字列からIPアドレスの変換に失敗");
    return -1;
  }
  if (!WiFi.config(local_ip, gateway, subnet, gateway)) {
    ESP_LOGE(TAG, "IPアドレスの設定に失敗");
    return -1;
  }
  if (WiFi.begin(p->wifiSsid, p->wifiPass) != WL_DISCONNECTED) {
    ESP_LOGE(TAG, "Not Status WL_DISCONNECTED");
    ESP.restart();
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  ESP_LOGI(TAG, "WL_CONNECTED");

  return 0;
}

int begin2AP() {
  WiFi.mode(WIFI_AP);
  IPAddress local_ip, subnet;
  if ((char2IPAddress(&local_ip, CONF_MODE_IP) != 0) || (char2IPAddress(&subnet, CONF_AP_SUBNET) != 0)) {
    ESP_LOGE(TAG, "文字列からIPアドレスの変換に失敗");
    return -1;
  }
  if (!WiFi.softAPConfig(local_ip, local_ip, subnet)) {
    ESP_LOGE(TAG, "APの設定に失敗");
    return -1;
  }
  if (!WiFi.softAP(CONF_AP_SSID, CONF_AP_PASS)) {
    ESP_LOGE(TAG, "APのスタートに失敗");
    return -1;
  }

  ESP_LOGI(TAG, "--- Soft AP Start ---");
  ESP_LOGI(TAG, "Soft AP SSID: " CONF_AP_SSID);
  ESP_LOGI(TAG, "Soft AP PASS: " CONF_AP_PASS);
  ESP_LOGI(TAG, "Soft AP MYIP: " CONF_MODE_IP);
  ESP_LOGI(TAG, "Soft AP SUBNET: " CONF_AP_SUBNET);

  return 0;
}

void scanWiFiAP(char *buf, size_t buf_len) {
  int i   = 0;
  int max = 10;
  int n   = WiFi.scanNetworks();
  if (n == 0) {
    sprintf(buf, "{\"data\": []}");
    ESP_LOGI(TAG, "No Networks found");
    return;
  }
  ESP_LOGI(TAG, "%d Networks found", n);
  DynamicJsonDocument doc(buf_len + 512);
  JsonArray           root = doc.createNestedArray("data");

  while ((i < n) && (i < max)) {
    JsonObject obj = root.createNestedObject();
    obj["ssid"]    = WiFi.SSID(i).c_str();
    obj["rssi"]    = WiFi.RSSI(i);
    obj["ch"]      = WiFi.channel(i);
    obj["enc"]     = WiFi.encryptionType(i) == 0 ? 0 : 1;
    i++;
  }

  WiFi.scanDelete();  // free memory
  serializeJson(doc, buf, buf_len);
}

static int char2IPAddress(IPAddress *ip, const char *data) {
  bool rtn;
  rtn = ip->fromString(data);
  return rtn == true ? 0 : -1;
}