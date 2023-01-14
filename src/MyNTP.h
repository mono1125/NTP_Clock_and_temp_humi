#ifndef MY_NTP_H
#define MY_NTP_H

#include <Arduino.h>
#include "esp_sntp.h"

extern void beginNtp(uint32_t interval_ms = 60000);  // 1min

#endif