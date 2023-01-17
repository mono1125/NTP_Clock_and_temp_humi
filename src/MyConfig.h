#ifndef MY_CONFIG_H
#define MY_CONFIG_H

#include <Arduino.h>
#include <MyFileManage.h>

/* GPIO PIN for Config mode */
/* Dev Env */
#define CONF_PIN         35
#define CONF_AP_MODE_PIN 36
/* PCB */
// #define CONF_PIN         9
// #define CONF_AP_MODE_PIN 10

/* GPIO PIN for Config mode */

#define DEVICE_NAME "ESP32"

/* RUN | CONF State
  (INFO) Inner Pullup to CONF_PIN and CONF_AP_MODE_PIN
  State          | CONF_PIN | CONF_AP_MODE_PIN
  --------------------------------------------
  RUN            |    H     |        H
  CONF_WIFI_STA  |    L     |        H
  CONF_WIFI_AP   |    L     |        L
*/
typedef enum { RUN, CONF_WIFI_STA, CONF_WIFI_AP } operation_mode_t;

/* (送信モード) 構造体ConfigのsendMode送信モード
  sendMode |  MODE
  -------------------------------------------
    0      |  not setting (default value)
    1      |  TCP Only
    2      |  MQTT Only
    3      |  TCP and MQTT
*/
typedef enum { NOT_SETTING = 0, TCP_ONLY = 1, MQTT_ONLY = 2, TCP_AND_MQTT = 3 } send_mode_t;

/* 設定値用構造体 */
typedef struct {
  uint8_t     deviceId;
  const char *localIPAddress;
  const char *subnetMask;
  const char *gatewayAddress;
  uint8_t     useDhcp;
  uint8_t     sendMode;
  const char *targetIPAddress;
  uint16_t    targetPort;
  const char *wifiSsid;
  const char *wifiPass;
  const char *testPubTopic;
  const char *prodPubTopic;
  const char *devLogPubTopic;
  const char *confSubTopic;
} Config;

/* オペレーションモード設定用ピン初期化
  引数: なし
  責務: GPIOピンを初期化する
*/
extern void initOpeModePin();

/* オペレーションモード取得
  引数: なし
  戻り値: オペレーションモード operation_mode_t
  責務: GPIOピンを読みオペレーションモードを返す
*/
extern operation_mode_t getOpeMode();

/* 設定ファイルを取得する
  引数: 設定ファイルの内容を格納する配列 buf
        格納する配列長 buf_len
  戻り値: 0 -> 正常 -1 -> 異常
  責務: 設定ファイル(config.json or default_config.json)を読みその内容を格納する
*/
extern int getConfig(char *buf, size_t buf_len);

/* 設定パラメタをチェックする
  引数: 設定値が格納されたJSONDocument
  戻り値: 0 -> 正常 -1 -> 異常
  責務: 設定パラメタに欠けがないかをチェックする
*/
extern int checkConfigParams(JsonDocument &doc);

/* 構造体Configに値をセットする
  引数: 構造体 Config
  責務: 設定ファイルを読みその内容を構造体Configにセットする
*/
extern void setConfig(Config *p);

/* 構造体Configのメンバの値をコンソールに出力する
  引数: 構造体 Config
  責務: 構造体Configのメンバの値をコンソール出力する
*/
extern void printConfig(const Config *p);

/* 送信モードの判定をする
  引数: 構造体 Config
  戻り値: enum型 send_mode_t
  責務: 構造体 ConfigのsendModeの値から送信モードを返す
*/
extern send_mode_t getSendMode(const Config *p);

/* 文字列が空かどうかを判定する
  引数: 評価する文字列
  戻り値: 0 -> 空 -1 -> 空ではない
  責務: 文字列が空かどうかを判定する
*/
extern int isEmptyChar(const char *buf);

#endif