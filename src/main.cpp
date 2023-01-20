/* Dependency Info

CONFIGURATION: https://docs.platformio.org/page/boards/espressif32/esp32-s3-devkitc-1.html
PLATFORM: Espressif 32 (5.2.0) > Espressif ESP32-S3-DevKitC-1-N8 (8 MB QD, No PSRAM)
HARDWARE: ESP32S3 240MHz, 320KB RAM, 8MB Flash
PACKAGES:
 - framework-arduinoespressif32 @ 3.20005.220925 (2.0.5)
 - tool-esptoolpy @ 1.40201.0 (4.2.1)
 - toolchain-riscv32-esp @ 8.4.0+2021r2-patch3
 - toolchain-xtensa-esp32s3 @ 8.4.0+2021r2-patch3
LDF: Library Dependency Finder -> https://bit.ly/configure-pio-ldf

Dependency Graph
|-- ArduinoJson @ 6.20.0
|-- PubSubClient @ 2.8.0
|-- ESP Async WebServer @ 1.2.3+sha.f71e3d4
|   |-- AsyncTCP @ 1.1.1
|   |-- FS @ 2.0.0
|   |-- WiFi @ 2.0.0
|-- FS @ 2.0.0
|-- LittleFS @ 2.0.0
|   |-- FS @ 2.0.0
|-- Wire @ 2.0.0
|-- WiFiClientSecure @ 2.0.0
|   |-- WiFi @ 2.0.0
|-- WiFi @ 2.0.0

RAM:   [==        ]  16.2% (used 52964 bytes from 327680 bytes)
Flash: [===       ]  28.8% (used 962173 bytes from 3342336 bytes)
*/

/* Dependency Info (ESP32-DEVKITC)
CONFIGURATION: https://docs.platformio.org/page/boards/espressif32/esp32dev.html
PLATFORM: Espressif 32 (5.2.0) > Espressif ESP32 Dev Module
HARDWARE: ESP32 240MHz, 320KB RAM, 4MB Flash
PACKAGES:
 - framework-arduinoespressif32 @ 3.20005.220925 (2.0.5)
 - tool-esptoolpy @ 1.40201.0 (4.2.1)
 - toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch3
LDF: Library Dependency Finder -> https://bit.ly/configure-pio-ldf
LDF Modes: Finder ~ deep, Compatibility ~ soft

Dependency Graph
|-- ArduinoJson @ 6.20.0
|-- PubSubClient @ 2.8.0
|-- ESP Async WebServer @ 1.2.3+sha.f71e3d4
|   |-- AsyncTCP @ 1.1.1
|   |-- FS @ 2.0.0
|   |-- WiFi @ 2.0.0
|-- FS @ 2.0.0
|-- LittleFS @ 2.0.0
|   |-- FS @ 2.0.0
|-- Wire @ 2.0.0
|-- WiFiClientSecure @ 2.0.0
|   |-- WiFi @ 2.0.0
|-- WiFi @ 2.0.0

RAM:   [==        ]  16.2% (used 53008 bytes from 327680 bytes)
Flash: [========  ]  77.2% (used 1012353 bytes from 1310720 bytes)
*/

#include <Arduino.h>
#include "MyConfig.h"
#include "MyFileManage.h"
#include "MyI2C.h"
#include "MyLed.h"
#include "MyMqtt.h"
#include "MyNTP.h"
#include "MyWebSrv.h"
#include "MyWiFi.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "secrets.h"

static char TAG[] = "main";

/* Interrupt */
volatile int timeCounter1;
volatile int timeCounter2;
hw_timer_t  *timer1   = NULL;  // measurement
hw_timer_t  *timer2   = NULL;  // display
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer1() {
  // ISR
  portENTER_CRITICAL_ISR(&timerMux);
  timeCounter1++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR onTimer2() {
  // ISR
  portENTER_CRITICAL_ISR(&timerMux);
  timeCounter2++;
  portEXIT_CRITICAL_ISR(&timerMux);
}
/* Interrupt */

/* Task Prototype */
void Task0a(void *pvParams);
void Task1a(void *pvParams);
/* Task Prototype */

static float humi = 0;
static float temp = 0;

struct tm timeInfo;
Config    config;

static void runMode(Config *config) {
  if (begin2STAForRUN(config) == 0) {
    beginNtp(60000);
    initMqtt(config);
    myWebSrv();

    xTaskCreatePinnedToCore(WiFiKeepAliveTask, "WiFi KeepAliveTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(mqttTask, "mqttTask", 8196, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(mqttRevMsgHandleTask, "mqttRevMsgHandleTask", 4096, NULL, 2, NULL, 0);
  } else {
    ESP_LOGE(TAG, "Wi-Fi ERROR. will restart...");
    delay(3000);
    ESP.restart();
  }

  initLedDisplay();
  myI2Cbegin();

  xTaskCreatePinnedToCore(Task0a, "Task0a", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(Task1a, "Task1a", 4096, NULL, 1, NULL, 1);

  ESP_LOGI(TAG, "RUN MODE START");
}

static void confSTAMode(Config *config) {
  if (begin2STAForCONFIG(config) == 0) {
    myWebSrv();
    xTaskCreatePinnedToCore(WiFiKeepAliveTask, "WiFi KeepAliveTask", 4096, NULL, 1, NULL, 0);
  } else {
    ESP_LOGE(TAG, "Wi-Fi ERROR. will restart...");
    delay(3000);
    ESP.restart();
  }
}

static void confAPMode() {
  if (begin2AP() == 0) {
    myWebSrv();
  } else {
    ESP_LOGE(TAG, "Wi-Fi ERROR. will restart...");
    delay(3000);
    ESP.restart();
  }
}

void setup() {
  initOpeModePin();
  initMyFileManage();
  setConfig(&config);
  printConfig(&config);
  ESP_LOGI(TAG, "sendMode: %d", getSendMode(&config));

  switch (getOpeMode()) {
    case RUN:
      runMode(&config);
      break;
    case CONF_WIFI_STA:
      confSTAMode(&config);
      break;
    case CONF_WIFI_AP:
      confAPMode();
      break;
    default:
      ESP_LOGE(TAG, "getOpeMode Error");
      break;
  }
}

static void pubHeapSize() {
  static MQTTData pub_heap;
  struct tm       _timeInfo;
  time_t          now;
  if (!getLocalTime(&_timeInfo, 5000)) {
    return;
  }
  unsigned long long unixTime = time(&now);
  strncpy(pub_heap.topic, DEVICE_FREE_HEAP_PUB_TOPIC, sizeof(pub_heap.topic) - 1);
  sprintf(
      pub_heap.data,
      "{\"time_stamp\": \"%04d-%02d-%02d %02d:%02d:%02d.000\",\"time_serial\": \"%llu\",\"val\": {\"heap_free\": %lu}}",
      (_timeInfo.tm_year + 1900), (_timeInfo.tm_mon + 1), (_timeInfo.tm_mday), (_timeInfo.tm_hour), (_timeInfo.tm_min),
      (_timeInfo.tm_sec), unixTime * 1000, esp_get_free_heap_size());
  xQueueSend(pubQueue, &pub_heap, 0);
}

static void pubCpuTemp() {
  static MQTTData pub_cpu_temp;
  struct tm       _timeInfo;
  time_t          now;
  if (!getLocalTime(&_timeInfo, 5000)) {
    return;
  }
  unsigned long long unixTime = time(&now);
  strncpy(pub_cpu_temp.topic, DEVICE_CPU_TEMP_PUB_TOPIC, sizeof(pub_cpu_temp.topic) - 1);
  sprintf(
      pub_cpu_temp.data,
      "{\"time_stamp\": \"%04d-%02d-%02d %02d:%02d:%02d.000\",\"time_serial\": \"%llu\",\"val\": {\"cpu_temp\": %.2f}}",
      (_timeInfo.tm_year + 1900), (_timeInfo.tm_mon + 1), (_timeInfo.tm_mday), (_timeInfo.tm_hour), (_timeInfo.tm_min),
      (_timeInfo.tm_sec), unixTime * 1000, temperatureRead());
  xQueueSend(pubQueue, &pub_cpu_temp, 0);
}

static void pubHumiAndTemp(float h, float t) {
  static MQTTData pub_humi_and_temp;
  struct tm       _timeInfo;
  time_t          now;
  if (!getLocalTime(&_timeInfo, 5000)) {
    return;
  }
  unsigned long long unixTime = time(&now);
  if (config.pubProd == 0) {
    strncpy(pub_humi_and_temp.topic, TEST_PUB_TOPIC, sizeof(pub_humi_and_temp.topic) - 1);
  } else {
    strncpy(pub_humi_and_temp.topic, PROD_PUB_TOPIC, sizeof(pub_humi_and_temp.topic) - 1);
  }
  sprintf(pub_humi_and_temp.data,
          "{\"time_stamp\": \"%04d-%02d-%02d %02d:%02d:%02d.000\",\"time_serial\": \"%llu\",\"val\": {\"humi\":%.2f, "
          "\"temp\":%.2f}}",
          (_timeInfo.tm_year + 1900), (_timeInfo.tm_mon + 1), (_timeInfo.tm_mday), (_timeInfo.tm_hour),
          (_timeInfo.tm_min), (_timeInfo.tm_sec), unixTime * 1000, h, t);
  xQueueSend(pubQueue, &pub_humi_and_temp, 0);
}

void loop() {
  ESP_LOGI("loop", "free heap size: %d", esp_get_free_heap_size());
  pubHeapSize();
  pubCpuTemp();
  delay(15000);
}

void Task0a(void *pvParams) {
  timer1 = timerBegin(0, getApbFrequency() / 1000000, true);  // 1us
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 10000000, true);
  timerAlarmEnable(timer1);
  while (true) {
    if (timeCounter1 > 0) {
      portENTER_CRITICAL(&timerMux);
      timeCounter1--;
      portEXIT_CRITICAL(&timerMux);
      if (myI2CGetData(&humi, &temp)) {
        ESP_LOGI("SENSOR", "Temp: %f, Humi: %f", temp, humi);
        pubHumiAndTemp(humi, temp);
      }
    }
    delay(100);
  }
  vTaskDelete(NULL);
}

void Task1a(void *pvParams) {
  timer2 = timerBegin(1, getApbFrequency() / 1000000, true);  // 1us
  timerAttachInterrupt(timer2, &onTimer2, true);
  timerAlarmWrite(timer2, 600, true);
  timerAlarmEnable(timer2);
  while (true) {
    if (timeCounter2 > 0) {
      portENTER_CRITICAL(&timerMux);
      timeCounter2--;
      portEXIT_CRITICAL(&timerMux);

      getLocalTime(&timeInfo);
      displayTime(&timeInfo, &humi, &temp);
    }
    delay(1);
  }
  vTaskDelete(NULL);
}