#include "MyMqtt.h"

static char TAG[] = "MyMqtt";

const char MQTT_SUB_TOPIC[] = "dev/" THING_NAME "/1";
const char MQTT_PUB_TOPIC[] = "test/" THING_NAME "/1";

WiFiClientSecure httpsClient;
PubSubClient mqttClient(httpsClient);

/*
参考
https://github.com/debsahu/ESP-MQTT-AWS-IoT-Core/blob/master/Arduino/PubSubClient/PubSubClient.ino
*/

static void mqttCallback(char* topic, byte* payload, unsigned int length){
  ESP_LOGI(TAG, "Message arrived");
  ESP_LOGI(TAG, "topic: %s", topic);
  char rev[500];
  if (length < (unsigned int)500){
    for (int i=0; i<length;i++){
      rev[i] = (char)payload[i];
    }
    ESP_LOGI(TAG, "%s", rev); // 一時バッファに格納すると扱える
  } else {
    ESP_LOGE(TAG, "Message length: %d is larger 500", length);
  }
}

static void pubSubErr(int8_t MQTTErr) {
  switch(MQTTErr) {
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

static void connectMqtt(){
  while(!mqttClient.connected()){
    if(mqttClient.connect(THING_NAME)){
      if (!mqttClient.subscribe(MQTT_SUB_TOPIC)){
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

void initMqtt(){
  httpsClient.setCACert(ROOT_CA);
  httpsClient.setCertificate(CLIENT_CERT);
  httpsClient.setPrivateKey(PRIVATE_KEY);
  mqttClient.setServer(MQTT_ENDPOINT, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  connectMqtt();
}

void mqttTask(void *pvParameters){
  while(1){
    mqttClient.loop();
    delay(3000);
  }
  vTaskDelete(NULL);
}