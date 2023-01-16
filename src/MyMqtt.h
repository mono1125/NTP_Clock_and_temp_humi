#ifndef MY_MQTT_H
#define MY_MQTT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "MyConfig.h"
#include "MyFileManage.h"
#include "esp_log.h"
#include "secrets.h"

#ifdef MQTT_MAX_PACKET_SIZE
#undef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 1024
#endif

#ifndef THING_NAME
#define THING_NAME "your thing name"
#endif
#ifndef MQTT_ENDPOINT
#define MQTT_ENDPOINT "xxxxxxx.iot.ap-northeast-1.amazonaws.com"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 8883
#endif

/* MQTT データ構造 */
typedef struct {
  char topic[50];
  char data[1024];
} MQTTData;

/* MQTTの初期設定をする
  引数: なし
  責務: MQTTの初期設定をする
    MQTT認証情報・エンドポイントの設定
    Subscribeしたときのコールバック関数の登録
    MQTT接続
    キューの作成
*/
extern void initMqtt(const Config* p);

extern void mqttTask(void* pvParameters);
extern void mqttRevMsgHandleTask(void* pvParameters);

/* トピックの設定をする
  引数: 構造体 Config
  責務: 設定値をもとにトピックを設定する
*/
static void initTopic(const Config* p);

/* キューの作成
  引数: なし
  責務: キューの作成をする
*/
static void initQueue();

static void connectMqtt();
static void pubSubErr(int8_t MQTTErr);
static void mqttCallback(char* topic, byte* payload, unsigned int length);
#endif