#include <Arduino.h>
#include "esp_log.h"
#include "MyWiFi.h"
#include "MyI2C.h"
#include "MyLed.h"

/* Interrupt */
volatile int timeCounter1;
volatile int timeCounter2;
hw_timer_t *timer1 = NULL; // measurement
hw_timer_t *timer2 = NULL; // display
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer1(){
    // ISR
    portENTER_CRITICAL_ISR(&timerMux);
    timeCounter1++;
    portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR onTimer2(){
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

void setup() {
  if (watchWiFiStatus() == 0) {
    Serial.println("ntp config");
    configTime(9 * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");
  }

  initLedDisplay();

  myI2Cbegin();

  xTaskCreatePinnedToCore(Task0a, "Task0a", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(Task1a, "Task1a", 4096, NULL, 1, NULL, 1);
}

void loop() {
  delay(1);
}

void Task0a(void *pvParams) {
  timer1 = timerBegin(0, getApbFrequency()/1000000, true); // 1us
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 10000000, true);
  timerAlarmEnable(timer1);
  while (true) {
    if (timeCounter1 > 0) {
      portENTER_CRITICAL(&timerMux);
      timeCounter1--;
      portEXIT_CRITICAL(&timerMux);
      if (myI2CGetData(&humi, &temp)){
        ESP_LOGI("SENSOR", "Temp: %f, Humi: %f", temp, humi);
      }
    }
    delay(100);
  }
  vTaskDelete(NULL);
}

void Task1a(void *pvParams) {
  timer2 = timerBegin(1, getApbFrequency()/1000000, true); // 1us
  timerAttachInterrupt(timer2, &onTimer2, true);
  timerAlarmWrite(timer2, 800, true);
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