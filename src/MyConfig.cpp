#include "MyConfig.h"

static const char TAG[] = "MyConfig";

void initOpeModePin() {
  pinMode(CONF_PIN, INPUT_PULLUP);
  pinMode(CONF_AP_MODE_PIN, INPUT_PULLUP);
}

operation_mode_t getOpeMode() {
  if (digitalRead(CONF_PIN)) {
    return RUN;
  } else if (digitalRead(CONF_AP_MODE_PIN)) {
    return CONF_WIFI_STA;
  } else {
    return CONF_WIFI_AP;
  }
}

int getConfig(char *buf, size_t buf_len) {
  if (getFile("/config.json", buf, buf_len) != 0) {
    if (getFile("/default_config.json", buf, buf_len) != 0) {
      ESP_LOGE(TAG, "Config File Error");
      return -1;
    }
  }
  return 0;
}

int checkConfigParams(JsonDocument &doc) {
  ESP_LOGI(TAG, "Start check config params...");
  if (!doc.containsKey("deviceName")) {
    ESP_LOGE(TAG, "Not Exists Key: deviceName");
    return -1;
  }
  if (!doc.containsKey("localIPAddress")) {
    ESP_LOGE(TAG, "Not Exists Key: localIPAddress");
    return -1;
  }
  if (!doc.containsKey("subnetMask")) {
    ESP_LOGE(TAG, "Not Exists Key: subnetMask");
    return -1;
  }
  if (!doc.containsKey("gatewayAddress")) {
    ESP_LOGE(TAG, "Not Exists Key: gatewayAddress");
    return -1;
  }
  if (!doc.containsKey("useDhcp")) {
    ESP_LOGE(TAG, "Not Exists Key: useDhcp");
    return -1;
  }
  if (!doc.containsKey("sendMode")) {
    ESP_LOGE(TAG, "Not Exists Key: sendMode");
    return -1;
  }
  if (!doc.containsKey("targetIPAddress")) {
    ESP_LOGE(TAG, "Not Exists Key: targetIPAddress");
    return -1;
  }
  if (!doc.containsKey("targetPort")) {
    ESP_LOGE(TAG, "Not Exists Key: targetPort");
    return -1;
  }
  if (!doc.containsKey("wifiSsid")) {
    ESP_LOGE(TAG, "Not Exists Key: wifiSsid");
    return -1;
  }
  if (!doc.containsKey("wifiPass")) {
    ESP_LOGE(TAG, "Not Exists Key: wifiPass");
    return -1;
  }
  ESP_LOGI(TAG, "Finish check config params!");
  return 0;
}

int checkLengthConfigParams(JsonDocument &doc) {
  uint16_t len_deviceId        = strlen(doc["deviceId"]);
  uint16_t len_localIPAddress  = strlen(doc["localIPAddress"]);
  uint16_t len_subnetMask      = strlen(doc["subnetMask"]);
  uint16_t len_gatewayAddress  = strlen(doc["gatewayAddress"]);
  uint16_t len_useDhcp         = strlen(doc["useDhcp"]);
  uint16_t len_sendMode        = strlen(doc["sendMode"]);
  uint16_t len_targetIPAddress = strlen(doc["targetIPAddress"]);
  uint16_t len_targetPort      = strlen(doc["targetPort"]);
  uint16_t len_wifiSsid        = strlen(doc["wifiSsid"]);
  uint16_t len_wifiPass        = strlen(doc["wifiPass"]);

  if (len_deviceId == 0 || len_deviceId > 2) {
    return -1;
  }
  if (len_localIPAddress > 15) {
    return -1;
  }
  if (len_subnetMask > 15) {
    return -1;
  }
  if (len_gatewayAddress > 15) {
    return -1;
  }
  if (len_useDhcp != 1) {
    return -1;
  }
  if (len_sendMode != 1) {
    return -1;
  }
  if (len_targetIPAddress > 15) {
    return -1;
  }
  if (len_targetPort > 5) {
    return -1;
  }
  if (len_wifiSsid > 33) {
    return -1;
  }
  if (len_wifiPass > 100) {
    return -1;
  }

  ESP_LOGI(TAG, "len deviceId: %d", len_deviceId);
  ESP_LOGI(TAG, "len localIPAddress: %d", len_localIPAddress);
  ESP_LOGI(TAG, "len subnetMask: %d", len_subnetMask);
  ESP_LOGI(TAG, "len gatewayAddress: %d", len_gatewayAddress);
  ESP_LOGI(TAG, "len useDhcp: %d", len_useDhcp);
  ESP_LOGI(TAG, "len sendMode: %d", len_sendMode);
  ESP_LOGI(TAG, "len targetIPAddress: %d", len_targetIPAddress);
  ESP_LOGI(TAG, "len targetPort: %d", len_targetPort);
  ESP_LOGI(TAG, "len wifiSsid: %d", len_wifiSsid);
  ESP_LOGI(TAG, "len wifiPass: %d", len_wifiPass);

  return 0;
}

void setConfig(Config *p) {
  DynamicJsonDocument doc(2048);
  if (getJsonObj("/config.json", doc) != 0) {
    if (getJsonObj("/default_config.json", doc) != 0) {
      ESP_LOGE(TAG, "設定ファイルエラー");
      return;
    }
  }
  const char *device_name = doc["deviceName"];
  if (strcmp(DEVICE_NAME, device_name) != 0) {
    ESP_LOGE(TAG, "DEVICE_NAMEと設定値のdeviceNameが一致しません");
    if (getJsonObj("/default_config.json", doc) != 0) {
      ESP_LOGE(TAG, "設定ファイルエラー");
      return;
    }
  }

  p->deviceId        = doc["deviceId"];
  p->localIPAddress  = doc["localIPAddress"];
  p->subnetMask      = doc["subnetMask"];
  p->gatewayAddress  = doc["gatewayAddress"];
  p->useDhcp         = doc["useDhcp"];
  p->sendMode        = doc["sendMode"];
  p->targetIPAddress = doc["targetIPAddress"];
  p->targetPort      = doc["targetPort"];
  p->wifiSsid        = doc["wifiSsid"];
  p->wifiPass        = doc["wifiPass"];
}

void printConfig(const Config *p) {
  ESP_LOGI(TAG, "(Config) deviceId: %d", p->deviceId);
  ESP_LOGI(TAG, "(Config) localIPAddress: %s", p->localIPAddress);
  ESP_LOGI(TAG, "(Config) subnetMask: %s", p->subnetMask);
  ESP_LOGI(TAG, "(Config) gatewayAddress: %s", p->gatewayAddress);
  ESP_LOGI(TAG, "(Config) useDhcp: %d", p->useDhcp);
  ESP_LOGI(TAG, "(Config) sendMode: %d", p->sendMode);
  ESP_LOGI(TAG, "(Config) targetIPAddress: %s", p->targetIPAddress);
  ESP_LOGI(TAG, "(Config) targetPort: %d", p->targetPort);
  ESP_LOGI(TAG, "(Config  wifiSsid: %s", p->wifiSsid);
  ESP_LOGI(TAG, "(Config  wifiPass: %s", p->wifiPass);
}

send_mode_t getSendMode(const Config *p) {
  uint8_t mode = p->sendMode;
  if (mode == 0) {
    return NOT_SETTING;
  } else if (mode == 1) {
    return TCP_ONLY;
  } else if (mode == 2) {
    return MQTT_ONLY;
  } else if (mode == 3) {
    return TCP_AND_MQTT;
  } else {
    return NOT_SETTING;
  }
}

int isEmptyChar(const char *buf) {
  if ((buf == NULL) || (strlen(buf) == 0)) {
    return 0;
  }
  return -1;
}