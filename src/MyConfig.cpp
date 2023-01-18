#include "MyConfig.h"

static const char TAG[] = "MyConfig";

/* 内部 */
static int checkNameConfigParams(JsonDocument &doc);
static int checkExistsConfigParams(JsonDocument &doc);
static int checkLengthConfigParams(JsonDocument &doc);
static int checkRangeConfigParams(JsonDocument &doc);
/* 内部 */

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

int checkParams(JsonDocument &doc) {
  if (checkNameConfigParams(doc) != 0) {
    return -1;
  }
  if (checkExistsConfigParams(doc) != 0) {
    return -2;
  }
  if (checkLengthConfigParams(doc) != 0) {
    return -3;
  }
  if (checkRangeConfigParams(doc) != 0) {
    return -4;
  }
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

  p->deviceId = doc["deviceId"];
  strncpy(p->localIPAddress, doc["localIPAddress"], IP_ADDRESS_LEN_MAX);
  strncpy(p->subnetMask, doc["subnetMask"], IP_ADDRESS_LEN_MAX);
  strncpy(p->gatewayAddress, doc["gatewayAddress"], IP_ADDRESS_LEN_MAX);
  p->useDhcp  = doc["useDhcp"];
  p->sendMode = doc["sendMode"];
  strncpy(p->targetIPAddress, doc["targetIPAddress"], IP_ADDRESS_LEN_MAX);
  p->targetPort = doc["targetPort"];
  strncpy(p->wifiSsid, doc["wifiSsid"], WIFI_SSID_LEN_MAX);
  strncpy(p->wifiPass, doc["wifiPass"], WIFI_PASS_LEN_MAX);
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

static int checkNameConfigParams(JsonDocument &doc) {
  if (strcmp(DEVICE_NAME, doc["deviceName"]) != 0) {
    return -1;
  }
  return 0;
}

static int checkExistsConfigParams(JsonDocument &doc) {
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

static int checkLengthConfigParams(JsonDocument &doc) {
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
  if (len_localIPAddress > IP_ADDRESS_LEN_MAX) {
    return -1;
  }
  if (len_subnetMask > IP_ADDRESS_LEN_MAX) {
    return -1;
  }
  if (len_gatewayAddress > IP_ADDRESS_LEN_MAX) {
    return -1;
  }
  if (len_useDhcp != 1) {
    return -1;
  }
  if (len_sendMode != 1) {
    return -1;
  }
  if (len_targetIPAddress > IP_ADDRESS_LEN_MAX) {
    return -1;
  }
  if (len_targetPort > 5) {
    return -1;
  }
  if (len_wifiSsid > WIFI_SSID_LEN_MAX) {
    return -1;
  }
  if (len_wifiPass > WIFI_PASS_LEN_MAX) {
    return -1;
  }
  return 0;
}

static int checkRangeConfigParams(JsonDocument &doc) {
  int val_deviceId   = doc["deviceId"];
  int val_useDhcp    = doc["useDhcp"];
  int val_sendMode   = doc["sendMode"];
  int val_targetPort = doc["targetPort"];
  if (val_deviceId < 1 || val_deviceId > 99) {
    return -1;
  }
  if (val_useDhcp < 0 || val_useDhcp > 1) {
    return -1;
  }
  if (val_sendMode < 1 || val_sendMode > 3) {
    return -1;
  }
  if (val_targetPort < 1 || val_targetPort > 65535) {
    return -1;
  }
  return 0;
}