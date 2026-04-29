#include "MyI2C.h"

#define TEMP_OFFSET (-5)
#define HUMI_OFFSET (-2)
// #define TEMP_OFFSET (-5)
// #define HUMI_OFFSET (-2)

static char TAG[] = "MyI2C";

void myI2Cbegin() {
  Wire.setPins(SDA_PIN, SCL_PIN);
  Wire.begin();
  delay(100);
  myI2CCheckStatus();
}

void myI2CCheckStatus() {
  Wire.beginTransmission(I2C_DEVIVE_ADDRESS);
  Wire.write(0x71);
  Wire.endTransmission();
  Wire.requestFrom(I2C_DEVIVE_ADDRESS, 1);
  uint8_t res = Wire.read();
  ESP_LOGI(TAG, "myI2CCheckStatus res: 0x%02X", res);

  if (res & 0x08) {
    ESP_LOGI(TAG, "status calibrated OK");
  } else {
    ESP_LOGW(TAG, "not calibrated, sending init command");
    uint8_t init_cmd[] = {0xBE, 0x08, 0x00};
    Wire.beginTransmission(I2C_DEVIVE_ADDRESS);
    Wire.write(init_cmd, 3);
    Wire.endTransmission();
    delay(10);
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
      if ((c & 0x80) == 0) {  // bit7=0 → measurement complete
        return 1;
      }
    }
    delay(20);
  }
}

int myI2CGetData(float *h, float *t) {
  if (!myI2CStartMeasure()) {
    ESP_LOGE(TAG, "measurement timeout");
    return 0;
  }
  Wire.requestFrom(I2C_DEVIVE_ADDRESS, 6);

  unsigned char str[6];
  int           index = 0;
  while (Wire.available()) {
    str[index++] = Wire.read();
  }

  if (str[0] & 0x80) {
    return 0;
  }

  unsigned long humi_raw = 0;
  unsigned long temp_raw = 0;

  humi_raw = str[1];
  humi_raw <<= 8;
  humi_raw += str[2];
  humi_raw <<= 4;
  humi_raw += str[3] >> 4;

  *h = (float)(((humi_raw / 1048576.0) * 100) + HUMI_OFFSET);

  temp_raw = str[3] & 0x0f;
  temp_raw <<= 8;
  temp_raw += str[4];
  temp_raw <<= 8;
  temp_raw += str[5];

  *t = (float)(((temp_raw / 1048576.0) * 200.0) - 50.0 + TEMP_OFFSET);

  return 1;
}
