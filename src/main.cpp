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
#include "MyTCP.h"
#include "MyWebSrv.h"
#include "MyWiFi.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "secrets.h"

static char TAG[] = "main";

/* Interrupt */
hw_timer_t                *meas_timer            = NULL;  // measurement
hw_timer_t                *led_timer             = NULL;  // display
volatile SemaphoreHandle_t meas_semaphore_handle = NULL;
volatile SemaphoreHandle_t led_semaphore_handle  = NULL;

void IRAM_ATTR onMeasTimer() {
  xSemaphoreGiveFromISR(meas_semaphore_handle, NULL);
}

void IRAM_ATTR onLedTimer() {
  xSemaphoreGiveFromISR(led_semaphore_handle, NULL);
}
/* Interrupt */

/* Task Prototype */
void sensorTask(void *pvParams);
void ledTask(void *pvParams);
/* Task Prototype */

static float humi = 0;
static float temp = 0;

struct tm timeInfo;
Config    config;

static void runMode(Config *config) {
  if (begin2STAForRUN(config) != 0) {
    ESP_LOGE(TAG, "Wi-Fi ERROR. will restart...");
    delay(3000);
    ESP.restart();
  }

  beginNtp(60000);
  myWebSrv();
  xTaskCreatePinnedToCore(WiFiKeepAliveTask, "WiFi KeepAliveTask", 4096, NULL, 1, NULL, 0);
  initLedDisplay();
  myI2Cbegin();
  xTaskCreatePinnedToCore(sensorTask, "Task0a", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(ledTask, "Task1a", 4096, NULL, 1, NULL, 1);

  switch (getSendMode(config)) {
    case TCP_ONLY:
      initTCPQueue();
      xTaskCreatePinnedToCore(TCPTask, "TCP Send Task", 4096, NULL, 2, NULL, 0);
      break;
    case MQTT_ONLY:
      initMqtt(config);
      xTaskCreatePinnedToCore(mqttTask, "mqttTask", 8196, NULL, 1, NULL, 0);
      xTaskCreatePinnedToCore(mqttRevMsgHandleTask, "mqttRevMsgHandleTask", 4096, NULL, 2, NULL, 0);
      break;
    case TCP_AND_MQTT:
      initTCPQueue();
      initMqtt(config);
      xTaskCreatePinnedToCore(TCPTask, "TCP Send Task", 4096, NULL, 2, NULL, 0);
      xTaskCreatePinnedToCore(mqttTask, "mqttTask", 8196, NULL, 1, NULL, 0);
      xTaskCreatePinnedToCore(mqttRevMsgHandleTask, "mqttRevMsgHandleTask", 4096, NULL, 2, NULL, 0);
    default:
      ESP_LOGE(TAG, "Send Mode Error");
      delay(3000);
      break;
  }
  ESP_LOGI(TAG, "RUN MODE START");
}

static void confSTAMode(Config *config) {
  if (begin2STAForCONFIG(config) != 0) {
    ESP_LOGE(TAG, "Wi-Fi ERROR. will restart...");
    delay(3000);
    ESP.restart();
  }
  myWebSrv();
  xTaskCreatePinnedToCore(WiFiKeepAliveTask, "WiFi KeepAliveTask", 4096, NULL, 1, NULL, 0);
}

static void confAPMode() {
  if (begin2AP() != 0) {
    ESP_LOGE(TAG, "Wi-Fi ERROR. will restart...");
    delay(3000);
    ESP.restart();
  }
  myWebSrv();
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

static void pubDeviceHealth() {
  static MQTTData pub_heap;
  struct tm       _timeInfo;
  time_t          now;
  if (!getLocalTime(&_timeInfo, 5000)) {
    return;
  }
  unsigned long unixTime = time(&now);
  strncpy(pub_heap.topic, DEVICE_HEALTH_PUB_TOPIC, sizeof(pub_heap.topic) - 1);
  sprintf(pub_heap.data,
          "{\"time_stamp\": \"%04d-%02d-%02d %02d:%02d:%02d\",\"time_serial\": "
          "\"%lu\",\"cpu_temp\":%.2f,\"heap_free\": %lu,\"rssi\":%d}",
          (_timeInfo.tm_year + 1900), (_timeInfo.tm_mon + 1), (_timeInfo.tm_mday), (_timeInfo.tm_hour),
          (_timeInfo.tm_min), (_timeInfo.tm_sec), unixTime, temperatureRead(), esp_get_free_heap_size(), WiFi.RSSI());
  xQueueSend(pubQueue, &pub_heap, 0);
}

static void pubHumiAndTemp(float h, float t) {
  static MQTTData pub_humi_and_temp;
  struct tm       _timeInfo;
  time_t          now;
  if (!getLocalTime(&_timeInfo, 5000)) {
    return;
  }
  unsigned long unixTime = time(&now);
  if (config.pubProd == 0) {
    strncpy(pub_humi_and_temp.topic, TEST_PUB_TOPIC, sizeof(pub_humi_and_temp.topic) - 1);
  } else {
    strncpy(pub_humi_and_temp.topic, PROD_PUB_TOPIC, sizeof(pub_humi_and_temp.topic) - 1);
  }
  sprintf(pub_humi_and_temp.data,
          "{\"time_stamp\": \"%04d-%02d-%02d %02d:%02d:%02d\",\"time_serial\": \"%lu\",\"humi\":%.2f, "
          "\"temp\":%.2f}",
          (_timeInfo.tm_year + 1900), (_timeInfo.tm_mon + 1), (_timeInfo.tm_mday), (_timeInfo.tm_hour),
          (_timeInfo.tm_min), (_timeInfo.tm_sec), unixTime, h, t);
  xQueueSend(pubQueue, &pub_humi_and_temp, 0);
}

static void tcpDeviceHealth() {
  static char buf[TCP_MSG_SIZE];
  struct tm   _timeInfo;
  time_t      now;
  if (!getLocalTime(&_timeInfo, 5000)) {
    return;
  }
  unsigned long unixTime = time(&now);
  sprintf(buf,
          "{\"time_stamp\": \"%04d-%02d-%02d %02d:%02d:%02d\",\"time_serial\": "
          "\"%lu\",\"cpu_temp\":%.2f,\"heap_free\": %lu,\"rssi\":%d}",
          (_timeInfo.tm_year + 1900), (_timeInfo.tm_mon + 1), (_timeInfo.tm_mday), (_timeInfo.tm_hour),
          (_timeInfo.tm_min), (_timeInfo.tm_sec), unixTime, temperatureRead(), esp_get_free_heap_size(), WiFi.RSSI());
  xQueueSend(tcpQueue, buf, 0);
}

static void tcpHumiAndTemp(float h, float t) {
  static char buf[TCP_MSG_SIZE];
  struct tm   _timeInfo;
  time_t      now;
  if (!getLocalTime(&_timeInfo, 5000)) {
    return;
  }
  unsigned long unixTime = time(&now);
  sprintf(buf,
          "{\"time_stamp\": \"%04d-%02d-%02d %02d:%02d:%02d\",\"time_serial\": \"%lu\",\"humi\":%.2f, "
          "\"temp\":%.2f}",
          (_timeInfo.tm_year + 1900), (_timeInfo.tm_mon + 1), (_timeInfo.tm_mday), (_timeInfo.tm_hour),
          (_timeInfo.tm_min), (_timeInfo.tm_sec), unixTime, h, t);

  xQueueSend(tcpQueue, buf, 0);
}

void loop() {
  ESP_LOGI("loop", "free heap size: %d", esp_get_free_heap_size());
  switch (getSendMode(&config)) {
    case TCP_ONLY:
      tcpDeviceHealth();
      tcpHumiAndTemp(humi, temp);
      break;
    case MQTT_ONLY:
      pubDeviceHealth();
      pubHumiAndTemp(humi, temp);
      break;
    case TCP_AND_MQTT:
      pubDeviceHealth();
      tcpDeviceHealth();
      pubHumiAndTemp(humi, temp);
      tcpHumiAndTemp(humi, temp);
      break;
    default:
      break;
  }
  delay(15000);
}

void sensorTask(void *pvParams) {
  meas_semaphore_handle = xSemaphoreCreateBinary();
  meas_timer            = timerBegin(0, getApbFrequency() / 1000000, true);  // 1us
  timerAttachInterrupt(meas_timer, &onMeasTimer, true);
  timerAlarmWrite(meas_timer, 10000000, true);
  timerAlarmEnable(meas_timer);
  while (true) {
    if (xSemaphoreTake(meas_semaphore_handle, 0) == pdTRUE) {
      if (myI2CGetData(&humi, &temp)) {
        ESP_LOGI("SENSOR", "Temp: %f, Humi: %f", temp, humi);
      }
    }
    delay(100);
  }
  vTaskDelete(NULL);
}

void ledTask(void *pvParams) {
  led_semaphore_handle = xSemaphoreCreateBinary();
  led_timer            = timerBegin(1, getApbFrequency() / 1000000, true);  // 1us
  timerAttachInterrupt(led_timer, &onLedTimer, true);
  timerAlarmWrite(led_timer, 600, true);
  timerAlarmEnable(led_timer);
  while (true) {
    if (xSemaphoreTake(led_semaphore_handle, 0) == pdTRUE) {
      getLocalTime(&timeInfo);
      displayTime(&timeInfo, &humi, &temp);
    }
    delay(1);
  }
  vTaskDelete(NULL);
}