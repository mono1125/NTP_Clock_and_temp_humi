#include "MyMqtt.h"

static char TAG[] = "MyMqtt";

const char MQTT_SUB_TOPIC[] = "dev/" THING_NAME "/1";
const char MQTT_PUB_TOPIC[] = "test/" THING_NAME "/1";

WiFiClientSecure httpsClient;
PubSubClient     mqttClient(httpsClient);
QueueHandle_t    subMsgQueue;
QueueHandle_t    pubMsgQueue;

/*
参考
https://github.com/debsahu/ESP-MQTT-AWS-IoT-Core/blob/master/Arduino/PubSubClient/PubSubClient.ino
*/

void initMqtt() {
  httpsClient.setCACert(ROOT_CA);
  httpsClient.setCertificate(CLIENT_CERT);
  httpsClient.setPrivateKey(PRIVATE_KEY);
  mqttClient.setServer(MQTT_ENDPOINT, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  connectMqtt();
  subMsgQueue = xQueueCreate(1, sizeof(char) * 500);
  pubMsgQueue = xQueueCreate(1, sizeof(char) * 500);
}

void mqttTask(void* pvParameters) {
  char buf[500];
  while (1) {
    mqttClient.loop();
    if (xQueueReceive(pubMsgQueue, &buf, 0) == pdPASS) {
      mqttClient.publish(MQTT_PUB_TOPIC, buf);
      ESP_LOGI(TAG, "publish %s", buf);  // 一時バッファに格納すると扱える
    }
    delay(3000);
  }
  vTaskDelete(NULL);
}

void mqttRevMsgHandleTask(void* pvParameters) {
  char buf[500];
  char msg[500] = "{\"message\":\"受け取りました\"}";
  while (1) {
    if (xQueueReceive(subMsgQueue, &buf, 0) == pdPASS) {
      ESP_LOGI(TAG, "%s", buf);  // 一時バッファに格納すると扱える
      xQueueSend(pubMsgQueue, msg, 0);
    }
    delay(100);
  }
  vTaskDelete(NULL);
}

static void mqttCallback(char* topic, byte* payload, unsigned int length) {
  ESP_LOGI(TAG, "Message arrived");
  ESP_LOGI(TAG, "topic: %s", topic);
  char rev[500];
  if (length < (unsigned int)500) {
    for (int i = 0; i < length; i++) {
      rev[i] = (char)payload[i];
    }
    ESP_LOGI(TAG, "%s", rev);  // 一時バッファに格納すると扱える
    xQueueSend(subMsgQueue, rev, 0);
  } else {
    ESP_LOGE(TAG, "Message length: %d is larger 500", length);
  }
}

static void connectMqtt() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect(THING_NAME)) {
      if (!mqttClient.subscribe(MQTT_SUB_TOPIC)) {
        pubSubErr(mqttClient.state());
      }
    } else {
      ESP_LOGE(TAG, "MQTT Connect Failed");
      pubSubErr(mqttClient.state());
      ESP_LOGI(TAG, "try again in 5 seconds");
      delay(5000);
    }
  }
}

static void pubSubErr(int8_t MQTTErr) {
  switch (MQTTErr) {
    case MQTT_CONNECTION_TIMEOUT:
      ESP_LOGE(TAG, "Connection tiemout");
      break;
    case MQTT_CONNECTION_LOST:
      ESP_LOGE(TAG, "Connection lost");
      break;
    case MQTT_CONNECT_FAILED:
      ESP_LOGE(TAG, "Connect failed");
      break;
    case MQTT_DISCONNECTED:
      ESP_LOGE(TAG, "Disconnected");
      break;
    case MQTT_CONNECTED:
      ESP_LOGI(TAG, "Connected");
      break;
    case MQTT_CONNECT_BAD_PROTOCOL:
      ESP_LOGE(TAG, "Connect bad protocol");
      break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
      ESP_LOGE(TAG, "Connect bad Client-ID");
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      ESP_LOGE(TAG, "Connect unavailable");
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      ESP_LOGE(TAG, "Connect bad credentials");
      break;
    case MQTT_CONNECT_UNAUTHORIZED:
      ESP_LOGE(TAG, "Connect unauthorized");
      break;
    default:
      ESP_LOGE(TAG, "default err");
      break;
  }
}
