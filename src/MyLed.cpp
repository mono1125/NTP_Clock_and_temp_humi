#include "MyLed.h"

static const int digits [] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00100111, // 7
  0b01111111, // 8
  0b01101111, // 9
};

static const int segmentPins [] = {
  OSL_A,
  OSL_B,
  OSL_C,
  OSL_D,
  OSL_E,
  OSL_F,
  OSL_G,
};

static const int a_digPins [] = {
  A_OSL_DIG6,
  A_OSL_DIG5,
  A_OSL_DIG4,
  A_OSL_DIG3,
  A_OSL_DIG2,
  A_OSL_DIG1,
};

static const int b_digPins [] = {
  B_OSL_DIG6,
  B_OSL_DIG5,
  B_OSL_DIG4,
  B_OSL_DIG3,
  B_OSL_DIG2,
  B_OSL_DIG1,
};

static const int c_digPins [] = {
  C_OSL_DIG6,
  C_OSL_DIG5,
  C_OSL_DIG4,
  C_OSL_DIG3,
  C_OSL_DIG2,
  C_OSL_DIG1,
};

static const int numOfSegmentPins = sizeof(segmentPins) / sizeof(segmentPins[0]); // セグメントの数
static const int numOfADigPins = sizeof(a_digPins) / sizeof(a_digPins[0]); // 桁数
static const int numOfBDigPins = sizeof(b_digPins) / sizeof(b_digPins[0]); // 桁数
static const int numOfCDigPins = sizeof(c_digPins) / sizeof(c_digPins[0]); // 桁数

void initLedDisplay() {
  for (int i=0; i<numOfSegmentPins; i++){
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], SEGMENT_OFF);
  }
  pinMode(OSL_DP, OUTPUT);
  digitalWrite(OSL_DP, SEGMENT_OFF);

  for (int i=0; i<numOfADigPins; i++){
    pinMode(a_digPins[i], OUTPUT);
    digitalWrite(a_digPins[i], DIGIT_OFF);
  }
  for (int i=0; i<numOfBDigPins; i++){
    pinMode(b_digPins[i], OUTPUT);
    digitalWrite(b_digPins[i], DIGIT_OFF);
  }
  for (int i=0; i<numOfCDigPins; i++){
    pinMode(c_digPins[i], OUTPUT);
    digitalWrite(c_digPins[i], DIGIT_OFF);
  }
}

static int getDigits(const double value, int m, int n) {
  /*****************************
   * valueのm桁目からn桁目を取得する
   * value：取得先となる浮動小数点数
   * m：取得を開始する桁
   * n：取得を終了する桁
   * 返却値：取得した桁（整数）
  *****************************/
  double tmp;
  double mod_value;
  int result;

  /* 事前に絶対値を求めておく */
  tmp = fabs(value);

  /* n桁目以下の桁を取得 */
  mod_value = fmod(tmp, pow(10, n + 1));

  /* m桁目以上の桁を取得 */
  result = mod_value / pow(10, m);

  return result;
}

// クリア
static void clearSegments() {
  for (int i=0; i<numOfSegmentPins; i++){
    digitalWrite(segmentPins[i], SEGMENT_OFF);
  }
}

// 数字を表示する
static void displayNumber(int n) {
  // digits[n]の各ビットを調べて対応するセグメントを点灯・消灯する
  for (int i=0; i< numOfSegmentPins; i++){
    digitalWrite(segmentPins[i], digits[n] & (1 << i) ? SEGMENT_ON : SEGMENT_OFF);
  }
}

// 6桁の数字を表示する
static void displayNumbers(SevenSegmentID id, int n) {
  switch (id){
    case A:
      for (int i = 0; i < numOfADigPins; i++) {  // 右の桁からディジットを選択する
        digitalWrite(a_digPins[i], DIGIT_ON);        // ディジットをオンにする
        displayNumber(n % 10);                       // 10で割った余りを求めて、1の位を求め、表示する
        delay(1);
        clearSegments();                        // セグメントをすべてオフにする
        digitalWrite(a_digPins[i], DIGIT_OFF);  // ディジットをオフにする
        n /= 10;                                // 10で割り、次に表示する数字を、1の位に移す
      }
      break;
    case B:
      for (int i = 0; i < numOfBDigPins; i++) {  // 右の桁からディジットを選択する
        digitalWrite(b_digPins[i], DIGIT_ON);        // ディジットをオンにする
        displayNumber(n % 10);                       // 10で割った余りを求めて、1の位を求め、表示する
        delay(1);
        clearSegments();                        // セグメントをすべてオフにする
        digitalWrite(b_digPins[i], DIGIT_OFF);  // ディジットをオフにする
        n /= 10;                                // 10で割り、次に表示する数字を、1の位に移す
      }
      break;
    default:
      break;
  }
}

// 時刻と温湿度を表示する
void displayTime(const tm *timeInfo, const float *humi, const float *temp) {
  int d = (((timeInfo->tm_year + 1900) - 2000)*10000) + ((timeInfo->tm_mon +1) * 100) + (timeInfo->tm_mday);
  int n = (timeInfo->tm_hour * 10000) + (timeInfo->tm_min * 100) + (timeInfo->tm_sec);

  // 整数3桁に変換する
  int tmp_humi = getDigits((double)*humi, -1, 1);
  int tmp_temp = getDigits((double)*temp, -1, 1);
  int ht = (tmp_temp * 1000) + (tmp_humi);

  for (int i = 0; i < numOfADigPins; i++) {  // 右の桁からディジットを選択する
    digitalWrite(a_digPins[i], DIGIT_ON);        // ディジットをオンにする
    displayNumber(d % 10);                       // 10で割った余りを求めて、1の位を求め、表示する
    delayMicroseconds(100);
    // delay(1);
    clearSegments();                        // セグメントをすべてオフにする

    if ((i == 2) || (i == 4)){
      digitalWrite(OSL_DP, SEGMENT_ON);
      delayMicroseconds(100);
      // delay(1);
      digitalWrite(OSL_DP, SEGMENT_OFF);
    }
    digitalWrite(a_digPins[i], DIGIT_OFF);  // ディジットをオフにする
    d /= 10;                                // 10で割り、次に表示する数字を、1の位に移す
  }
  for (int i = 0; i < numOfBDigPins; i++) {  // 右の桁からディジットを選択する
    digitalWrite(b_digPins[i], DIGIT_ON);        // ディジットをオンにする
    displayNumber(n % 10);                       // 10で割った余りを求めて、1の位を求め、表示する
    delayMicroseconds(100);
    // delay(1);
    clearSegments();                        // セグメントをすべてオフにする

    if ((i == 2) || (i == 4)){
      digitalWrite(OSL_DP, SEGMENT_ON);
      delayMicroseconds(100);
      // delay(1);
      digitalWrite(OSL_DP, SEGMENT_OFF);
    }
    digitalWrite(b_digPins[i], DIGIT_OFF);  // ディジットをオフにする
    n /= 10;                                // 10で割り、次に表示する数字を、1の位に移す
  }
  for (int i = 0; i < numOfCDigPins; i++) {  // 右の桁からディジットを選択する
    digitalWrite(c_digPins[i], DIGIT_ON);        // ディジットをオンにする
    displayNumber(ht % 10);                       // 10で割った余りを求めて、1の位を求め、表示する
    delayMicroseconds(100);
    // delay(1);
    clearSegments();                        // セグメントをすべてオフにする

    if ((i == 1) || (i == 4)){
      digitalWrite(OSL_DP, SEGMENT_ON);
      delayMicroseconds(100);
      // delay(1);
      digitalWrite(OSL_DP, SEGMENT_OFF);
    }
    digitalWrite(c_digPins[i], DIGIT_OFF);  // ディジットをオフにする
    ht /= 10;                                // 10で割り、次に表示する数字を、1の位に移す
  }
}