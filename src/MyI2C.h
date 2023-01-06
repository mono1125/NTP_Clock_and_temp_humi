#ifndef MY_I2C_H
#define MY_I2C_H
#include <Arduino.h>
#include <Wire.h>
#include "esp_log.h"

/* I2C PIN*/
// Device address 0x38
#define SCL_PIN 46
#define SDA_PIN 3
#define I2C_DEVIVE_ADDRESS (0x38) // 0x38
/* I2C PIN*/

extern void myI2Cbegin();
extern void myI2CCheckStatus();
extern int myI2CGetData(float *h, float *t);

#endif