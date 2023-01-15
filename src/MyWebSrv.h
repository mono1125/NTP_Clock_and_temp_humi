#ifndef MY_WEB_SRV_H
#define MY_WEB_SRV_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "MyConfig.h"
#include "MyFileManage.h"
#include "MyWiFi.h"
#include "secrets.h"

#ifndef HTTP_PORT
#define HTTP_PORT 80
#endif
#ifndef HTTP_USER
#define HTTP_USER "admin"
#endif
#ifndef HTTP_PASS
#define HTTP_PASS "passw0rd"
#endif
#ifndef THING_NAME
#define THING_NAME "default thing (aws iot)"
#endif

extern void myWebSrv();

static void printWebSrvInfo();

#endif