#ifndef MY_FILE_MANAGE_H
#define MY_FILE_MANAGE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "FS.h"
#include "esp_log.h"

/*
  参考
  https://github.com/espressif/arduino-esp32/blob/master/libraries/LittleFS/examples/LITTLEFS_PlatformIO/src/main.cpp
*/

#define FORMAT_LITTLEFS_IF_FAILED true

/* LittleFSを開始する
  引数: なし
  責務: LittleFSを開始する
*/
extern void initMyFileManage();

/* JSONファイルを読みJSONオブジェクトを取得する
  引数: JSONファイルのパス
      読んだ結果を格納するJSONオブジェクト
  戻り値: 0 -> 正常 -1-> 異常
  責務:
  指定されたパスのJSONファイルを読み、渡されたJSONオブジェクトに結果を格納する
*/
extern int getJsonObj(const char *path, JsonDocument &doc);

/* JSONオブジェクトを保存する
  引数: 保存するパス path
        保存するJSONオブジェクト doc
  戻り値: 0 -> 正常 -1-> 異常
  責務: JSONオブジェクトをシリアライズして保存する
*/
extern int writeJsonFile(const char *path, JsonDocument &doc);

/* 内部 */
/* リストする
  引数: File (LittleFS)
      リストするディレクトリ始点 dirname
      リストする深さ levels
  責務: ディレクトリ, ファイルをリストする
*/
static void listDir(fs::FS &fs, const char *dirname, uint8_t levels);

/* 指定したパスのファイルをコンソール出力する
  引数: File (LittleFS)
      出力するファイルパス path
  責務: 指定されたファイルを読みコンソールに出力する
*/
static void printFile(fs::FS &fs, const char *path);

/* ファイルを読む
  引数: File (LittleFS)
      読むファイルのパス(ルート"/"から記述する) path
      読んだ結果を格納するchar配列 buf
      読んだ結果を格納するchar配列の長さ (sizeof()で長さを渡す) buf_len
  戻り値: 0 -> 正常 -1-> 異常
  責務: 指定されたファイルを読み、その結果を配列に格納する
*/
static int readFile(fs::FS &fs, const char *path, char *buf, const size_t buf_len);

/* ファイルを書き込む
  引数: File (LittleFS)
    書き込むファイルのパス(ルート"/"から記述する) path
    書き込むデータ data
  戻り値: 0 -> 正常 -1-> 異常
  責務: 指定されたパスにデータを書き込む
*/
static int writeFile(fs::FS &fs, const char *path, const char *data);

/* 外部 */
/* --- JSON操作用 --- */
/* 文字列をデシリアライズしてJSONオブジェクトに格納する
  引数: デシリアライズする文字列 data
        デシリアライズした結果を格納するJSONオブジェクト doc
  戻り値: 0 -> 正常 -1-> 異常
  責務: 文字列をデシリアライズしてJSONオブジェクトに格納する
*/
extern int myDeserializeJson(JsonDocument &doc, char *data);

#endif