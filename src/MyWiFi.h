#ifndef MY_WIFI_H
#define MY_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_log.h"
#include "secrets.h"

#ifndef WIFI_SSID
#define WIFI_SSID "Your Wi-Fi SSID"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "Your Wi-Fi PASSWORD"
#endif

// Wi-Fi接続状態監視
extern int watchWiFiStatus();

#endif