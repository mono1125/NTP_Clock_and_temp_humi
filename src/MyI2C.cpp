#include "MyI2C.h"

static char TAG[] = "MyI2C";

void myI2Cbegin() {
  Wire.setPins(SDA_PIN, SCL_PIN);
  Wire.begin();
  delay(100);
}

void myI2CCheckStatus() {
  static uint8_t res = 0;

  Wire.beginTransmission(I2C_DEVIVE_ADDRESS);
  Wire.write(0x71);
  Wire.endTransmission();
  Wire.requestFrom(I2C_DEVIVE_ADDRESS, 1);
  res = Wire.read();
  ESP_LOGI(TAG, "myI2CCheckStatus res: %d", res);

  if (res == 0x18) {
    ESP_LOGI(TAG, "status word OK");
  }
}

static int myI2CStartMeasure() {
  uint8_t cmd[] = {0xAC, 0x33, 0x00};

  Wire.beginTransmission(I2C_DEVIVE_ADDRESS);
  Wire.write(cmd, 3);
  Wire.endTransmission();
  delay(50);

  unsigned long timer_s = millis();
  while (1) {
    if (millis() - timer_s > 200) {
      return 0;
    }
    Wire.requestFrom(I2C_DEVIVE_ADDRESS, 1);

    while (Wire.available()) {
      unsigned char c = Wire.read();
      if ((c & 0x80) != 0) {
        return 1;
      }
    }
    delay(20);
  }
}

int myI2CGetData(float *h, float *t) {
  myI2CStartMeasure();
  Wire.requestFrom(I2C_DEVIVE_ADDRESS, 6);

  unsigned char str[6];
  int index = 0;
  while (Wire.available()) {
    str[index++] = Wire.read();
  }

  if(str[0] & 0x80) {
    return 0;
  }

  unsigned long __humi = 0;
  unsigned long __temp = 0;

  __humi = str[1];
  __humi <<= 8;
  __humi += str[2];
  __humi <<= 4;
  __humi += str[3] >> 4;

  *h = (float)((__humi/1048576.0) * 100);

  __temp = str[3]&0x0f;
  __temp <<=8;
  __temp += str[4];
  __temp <<=8;
  __temp += str[5];

  *t = (float)(((__temp/1048576.0) * 200.0) -50.0);

  return 1;
}
