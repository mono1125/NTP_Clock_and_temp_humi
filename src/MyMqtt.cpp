#include "MyMqtt.h"

static char TAG[] = "MyMqtt";

/* 内部 */
static void initQueue();
static void connectMqtt();
static void pubSubErr(int8_t MQTTErr);
static void clearMqttData(MQTTData* p, size_t topic_len, size_t data_len);
static void mqttCallback(char* topic, byte* payload, unsigned int length);
/* 内部 */

WiFiClientSecure              httpsClient;
PubSubClient                  mqttClient(httpsClient);
volatile QueueHandle_t        pubQueue = NULL;
static volatile QueueHandle_t subQueue = NULL;

/*
参考
https://github.com/debsahu/ESP-MQTT-AWS-IoT-Core/blob/master/Arduino/PubSubClient/PubSubClient.ino
*/

void initMqtt(const Config* p) {
  httpsClient.setCACert(ROOT_CA);
  httpsClient.setCertificate(CLIENT_CERT);
  httpsClient.setPrivateKey(PRIVATE_KEY);
  mqttClient.setServer(MQTT_ENDPOINT, MQTT_PORT);
  mqttClient.setBufferSize(2048);
  mqttClient.setCallback(mqttCallback);
  connectMqtt();
  initQueue();
  ESP_LOGI(TAG, "Buffer Size: %d", mqttClient.getBufferSize());
}

void mqttTask(void* pvParameters) {
  static MQTTData mqtt_data;
  while (1) {
    if (!mqttClient.loop()) {
      mqttClient.connect(THING_NAME);
    }
    if (xQueueReceive(pubQueue, &mqtt_data, 0) == pdPASS) {
      if (strcmp(mqtt_data.topic, TEST_PUB_TOPIC) == 0) {
        if (!mqttClient.publish(TEST_PUB_TOPIC, mqtt_data.data)) {
          ESP_LOGE(TAG, "publish error");
        }
        ESP_LOGI(TAG, "(publish) Topic: %s, Data: %s", mqtt_data.topic, mqtt_data.data);
      } else if (strcmp(mqtt_data.topic, PROD_PUB_TOPIC) == 0) {
        if (!mqttClient.publish(PROD_PUB_TOPIC, mqtt_data.data)) {
          ESP_LOGE(TAG, "publish error");
        }
        ESP_LOGI(TAG, "(publish) Topic: %s, Data: %s", mqtt_data.topic, mqtt_data.data);
      } else if (strcmp(mqtt_data.topic, DEVICE_CPU_TEMP_PUB_TOPIC) == 0) {
        if (!mqttClient.publish(DEVICE_CPU_TEMP_PUB_TOPIC, mqtt_data.data)) {
          ESP_LOGE(TAG, "publish error");
        }
        ESP_LOGI(TAG, "(publish) Topic: %s, Data: %s", mqtt_data.topic, mqtt_data.data);
      } else if (strcmp(mqtt_data.topic, DEVICE_FREE_HEAP_PUB_TOPIC) == 0) {
        if (!mqttClient.publish(DEVICE_FREE_HEAP_PUB_TOPIC, mqtt_data.data)) {
          ESP_LOGE(TAG, "publish error");
        }
        ESP_LOGI(TAG, "(publish) Topic: %s, Data: %s", mqtt_data.topic, mqtt_data.data);
      } else if (strcmp(mqtt_data.topic, DEVICE_RESPONSE_PUB_TOPIC) == 0) {
        if (!mqttClient.publish(DEVICE_RESPONSE_PUB_TOPIC, mqtt_data.data)) {
          ESP_LOGE(TAG, "publish error");
        }
        ESP_LOGI(TAG, "(publish) Topic: %s, Data: %s", mqtt_data.topic, mqtt_data.data);
      } else {
        ESP_LOGE(TAG, "(publish Error) Topic: %s, Data: %s", mqtt_data.topic, mqtt_data.data);
        // mqttClient.publish(mqtt_data.topic, mqtt_data.data);
      }
    }
    delay(2000);
  }
  vTaskDelete(NULL);
}

/* mqttCallback関数からデータが送られてくる
  コールバック関数で受け取った内容を処理するタスク
*/
void mqttRevMsgHandleTask(void* pvParameters) {
  static MQTTData receive_data;
  MQTTData        response_data;

  while (1) {
    if (xQueueReceive(subQueue, &receive_data, 0) == pdPASS) {
      /* get-config */
      if (strcmp(receive_data.topic, "CONF/" THING_NAME "/get-config") == 0) {
        clearMqttData(&response_data, sizeof(response_data.topic), sizeof(response_data.data));  // Important!!
        strncpy(response_data.topic, DEVICE_RESPONSE_PUB_TOPIC, sizeof(response_data.topic) - 1);
        // if (getFile("/config.json", response_data.data, sizeof(response_data.data)) == 0) {
        if (getFile("/new_config.json", response_data.data, sizeof(response_data.data)) == 0) {
          xQueueSend(pubQueue, &response_data, 0);
        } else {
          strncpy(response_data.data, "{\"message\": \"Failed get config file\"}", sizeof(response_data.data) - 1);
          xQueueSend(pubQueue, &response_data, 0);
        }
      }

      /* update config */
      if (strcmp(receive_data.topic, "CONF/" THING_NAME "/set-config") == 0) {
        clearMqttData(&response_data, sizeof(response_data.topic), sizeof(response_data.data));
        strncpy(response_data.topic, DEVICE_RESPONSE_PUB_TOPIC, sizeof(response_data.topic) - 1);
        DynamicJsonDocument doc(2048);
        if (myDeserializeJson(doc, receive_data.data) == 0) {
          switch (checkParams(doc)) {
            case 0:
              // if (writeJsonFile("/config.json", doc) != 0) {
              if (writeJsonFile("/new_config.json", doc) != 0) {
                strncpy(response_data.data, "{\"message\": \"Failed config update\"}", sizeof(response_data.data) - 1);
                break;
              }
              strncpy(response_data.data, "{\"message\": \"config updated!\"}", sizeof(response_data.data) - 1);
              break;
            case -1:
              strncpy(response_data.data, "{\"message\": \"Invalid paramater! Error: check name config params\"}",
                      sizeof(response_data.data) - 1);
              break;
            case -2:
              strncpy(response_data.data, "{\"message\": \"Invalid paramater! Error: check exists config params\"}",
                      sizeof(response_data.data) - 1);
              break;
            case -3:
              strncpy(response_data.data, "{\"message\": \"Invalid paramater! Error: check length config params\"}",
                      sizeof(response_data.data) - 1);
              break;
            case -4:
              strncpy(response_data.data, "{\"message\": \"Invalid paramater! Error: check range config params\"}",
                      sizeof(response_data.data) - 1);
              break;
            default:
              ESP_LOGE(TAG, "UNKNOWN ERROR");
              break;
          }
        }
        xQueueSend(pubQueue, &response_data, 0);
      }

      /* reboot */
      if (strcmp(receive_data.topic, "CONF/" THING_NAME "/reboot") == 0) {
        ESP_LOGI(TAG, "Reboot Topic! will reboot after 3 seconds...");
        clearMqttData(&response_data, sizeof(response_data.topic), sizeof(response_data.data));
        strncpy(response_data.topic, DEVICE_RESPONSE_PUB_TOPIC, sizeof(response_data.topic) - 1);
        strncpy(response_data.data, "{\"message\": \"Receive Reboot Topic! will reboot ...\"}",
                sizeof(response_data.data) - 1);
        xQueueSend(pubQueue, &response_data, 0);
        delay(3000);
        ESP.restart();
      }

      ESP_LOGI(TAG, "topic: %s", receive_data.topic);
      ESP_LOGI(TAG, "data: %s", receive_data.data);
    }
    delay(100);
  }
  vTaskDelete(NULL);
}

static void clearMqttData(MQTTData* p, size_t topic_len, size_t data_len) {
  memset(p->topic, '\0', topic_len);
  memset(p->data, '\0', data_len);
}

static void mqttCallback(char* topic, byte* payload, unsigned int length) {
  ESP_LOGI(TAG, "Message arrived");
  ESP_LOGI(TAG, "topic: %s", topic);
  static MQTTData receive_data;
  if (length < sizeof(receive_data.data)) {
    for (int i = 0; i < length; i++) {
      receive_data.data[i] = (char)payload[i];
    }
    sprintf(receive_data.topic, "%s", topic);
    xQueueSend(subQueue, &receive_data, 0);
  } else {
    ESP_LOGE(TAG, "Message length: %d is too larger(buf) %d", length, sizeof(receive_data.data));
  }
}

static void initQueue() {
  pubQueue = xQueueCreate(5, sizeof(MQTTData));
  subQueue = xQueueCreate(2, sizeof(MQTTData));
  if (pubQueue == NULL || subQueue == NULL) {
    ESP_LOGE(TAG, "Queueの作成に失敗しました");
    delay(3000);
    ESP.restart();
  }
}

static void connectMqtt() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect(THING_NAME)) {
      if (!mqttClient.subscribe(CONF_SUB_TOPIC)) {
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
