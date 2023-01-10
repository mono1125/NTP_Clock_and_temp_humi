#ifndef MY_MQTT_H
#define MY_MQTT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "esp_log.h"
#include "secrets.h"

#ifndef THING_NAME
#define THING_NAME "your thing name"
#endif
#ifndef MQTT_ENDPOINT
#define MQTT_ENDPOINT "xxxxxxx.iot.ap-northeast-1.amazonaws.com"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 8883
#endif

extern void initMqtt();
extern void mqttTask(void *pvParameters);
extern void mqttRevMsgHandleTask(void *pvParameters);

#endif