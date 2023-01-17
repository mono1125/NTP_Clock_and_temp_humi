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
  if (!doc.containsKey("testPubTopic")) {
    ESP_LOGE(TAG, "Not Exists Key: testPubTopic");
    return -1;
  }
  if (!doc.containsKey("prodPubTopic")) {
    ESP_LOGE(TAG, "Not Exists Key: prodPubTopic");
    return -1;
  }
  if (!doc.containsKey("devLogPubTopic")) {
    ESP_LOGE(TAG, "Not Exists Key: devLogPubTopic");
    return -1;
  }
  if (!doc.containsKey("confSubTopic")) {
    ESP_LOGE(TAG, "Not Exists Key: confSubTopic");
    return -1;
  }
  ESP_LOGI(TAG, "Finish check config params!");
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
  p->testPubTopic    = doc["testPubTopic"];
  p->prodPubTopic    = doc["prodPubTopic"];
  p->devLogPubTopic  = doc["devLogPubTopic"];
  p->confSubTopic    = doc["confSubTopic"];
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
  ESP_LOGI(TAG, "(Config) testPubTopic: %s", p->testPubTopic);
  ESP_LOGI(TAG, "(Config) prodPubTopic: %s", p->prodPubTopic);
  ESP_LOGI(TAG, "(Config) devLogPubTopic: %s", p->devLogPubTopic);
  ESP_LOGI(TAG, "(Config) ConfSubTopic: %s", p->confSubTopic);
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