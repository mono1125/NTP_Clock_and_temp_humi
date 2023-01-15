#ifndef MY_WIFI_H
#define MY_WIFI_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <MyConfig.h>
#include <WiFi.h>
#include "esp_log.h"
#include "secrets.h"

#ifndef CONF_MODE_IP
#define CONF_MODE_IP "192.168.2.254"
#endif
#ifndef CONF_AP_SSID
#define CONF_AP_SSID "myesp32"
#endif

#ifndef CONF_AP_PASS
#define CONF_AP_PASS "myesp32-pass"
#endif

#ifndef CONF_AP_SUBNET
#define CONF_AP_SUBNET "255.255.255.0"
#endif

extern int myWiFibegin();

/* 子機モードでのWi-Fi動作設定
  引数: 構造体 Config
  戻り値: 0 -> 正常 (接続OK) -1 -> 異常
  責務: 子機モード時のWi-Fi動作設定をする
*/
extern int begin2STAForRUN(const Config *p);
extern int begin2STAForCONFIG(const Config *p);

/* 親機モードのでWi-Fi動作設定
  引数: なし
  戻り値: 0 -> 正常 (動作開始) -1 -> 異常
  責務: 親機として振る舞う
*/
extern int begin2AP();

/* FreeRTOSタスク
  引数: pbParameters (未使用)
  責務: 定周期でのWi-Fi接続状態監視をして、切れていれば再接続を行う
*/
extern void WiFiKeepAliveTask(void *pvParameters);

/* 周囲のWi-Fi APをスキャンしてリストを返す
  引数: 結果を格納する配列 buf
      配列の長さ buf_len
  責務: 周囲のWi-FiをスキャンしてシリアライズされたJSON文字列を返す
  データ構造
    {
      "data": [
        {
          "ssid": "hogehogehogehoge",
          "rssi": -65,
          "ch": 20,
          "enc": 1
        }
      ]
    }
*/
extern void scanWiFiAP(char *buf, size_t buf_len);

/* 文字列のIPアドレスをIPAddress型に変換する
  引数: 変換した結果を格納する IPAddress
        変換したい文字列のIPアドレス
  戻り値: 0 -> 正常, -1 -> 異常
  責務: 文字列のIPアドレスをIPAddress型に変換する
*/
static int char2IPAddress(IPAddress *ip, const char *data);

/* IPAddress型を文字列に変換する
  IPAddressクラスにはtoStringメソッドがあるためそれを使う
  (使用例)
  IPAddress tmp(255,255,255,255);
  ESP_LOGI("MyWiFi", "IPAddress: %s", tmp.toString().c_str());
  -> IPAddress: 255.255.255.255
*/

#endif