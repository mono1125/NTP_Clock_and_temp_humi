#include "MyMqtt.h"

static char TAG[] = "MyMqtt";

const char MQTT_SUB_TOPIC[] = "dev/" THING_NAME "/1";
const char MQTT_PUB_TOPIC[] = "test/" THING_NAME "/1";

WiFiClientSecure espClient;
PubSubClient client(espClient);

/*
参考
https://github.com/debsahu/ESP-MQTT-AWS-IoT-Core/blob/master/Arduino/PubSubClient/PubSubClient.ino
*/

void mqttCallback(char* topic, byte* payload, unsigned int length){
    ESP_LOGI(TAG, "Message arrived");
    ESP_LOGI(TAG, "topic: %s", topic);
    for (int i=0; i<length;i++){
        ESP_LOGI(TAG, "%s", (char)payload[i]);
    }
}

void pubSubErr(int8_t MQTTErr) {
  if (MQTTErr == MQTT_CONNECTION_TIMEOUT)
    // Serial.print("Connection tiemout");
    ESP_LOGE(TAG, "Connection tiemout");
  else if (MQTTErr == MQTT_CONNECTION_LOST)
    // Serial.print("Connection lost");
    ESP_LOGE(TAG, "Connection lost");
  else if (MQTTErr == MQTT_CONNECT_FAILED)
    // Serial.print("Connect failed");
    ESP_LOGE(TAG, "Connect failed");
  else if (MQTTErr == MQTT_DISCONNECTED)
    // Serial.print("Disconnected");
    ESP_LOGE(TAG, "Disconnected");
  else if (MQTTErr == MQTT_CONNECTED)
    // Serial.print("Connected");
    ESP_LOGI(TAG, "Connected");
  else if (MQTTErr == MQTT_CONNECT_BAD_PROTOCOL)
    // Serial.print("Connect bad protocol");
    ESP_LOGE(TAG, "Connect bad protocol");
  else if (MQTTErr == MQTT_CONNECT_BAD_CLIENT_ID)
    // Serial.print("Connect bad Client-ID");
    ESP_LOGE(TAG, "Connect bad Client-ID");
  else if (MQTTErr == MQTT_CONNECT_UNAVAILABLE)
    // Serial.print("Connect unavailable");
    ESP_LOGE(TAG, "Connect unavailable");
  else if (MQTTErr == MQTT_CONNECT_BAD_CREDENTIALS)
    // Serial.print("Connect bad credentials");
    ESP_LOGE(TAG, "Connect bad credentials");
  else if (MQTTErr == MQTT_CONNECT_UNAUTHORIZED)
    // Serial.print("Connect unauthorized");
    ESP_LOGE(TAG, "Connect unauthorized");
}

