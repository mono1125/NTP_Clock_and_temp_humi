#ifndef MY_LED_H
#define MY_LED_H
#include <Arduino.h>
#include "esp_log.h"

// 桁数
#define DIGIT_ON  HIGH
#define DIGIT_OFF LOW
// 数字
#define SEGMENT_ON  LOW
#define SEGMENT_OFF HIGH

/* ---------- 7Segment LED PIN MAP ---------- */
#define OSL_A  4   // A, cathode
#define OSL_B  5   // B, cathode
#define OSL_C  6   // C, cathode
#define OSL_D  7   // D, cathode
#define OSL_E  15  // E, cathode
#define OSL_F  16  // F, cathode
#define OSL_G  17  // G, cathode
#define OSL_DP 18  // DP, cathode

/* PCB */
#define A_OSL_DIG1 43  // DIG 1, anode
#define A_OSL_DIG2 44  // DIG 2, anode
#define A_OSL_DIG3 1   // DIG 3, anode
#define A_OSL_DIG4 2   // DIG 4, anode
#define A_OSL_DIG5 42  // DIG 4, anode
#define A_OSL_DIG6 41  // DIG 4, anode

#define B_OSL_DIG1 40  // DIG 1, anode
#define B_OSL_DIG2 39  // DIG 2, anode
#define B_OSL_DIG3 38  // DIG 3, anode
#define B_OSL_DIG4 37  // DIG 4, anode
#define B_OSL_DIG5 36  // DIG 4, anode
#define B_OSL_DIG6 35  // DIG 4, anode

#define C_OSL_DIG1 0   // DIG 1, anode
#define C_OSL_DIG2 45  // DIG 2, anode
#define C_OSL_DIG3 48  // DIG 3, anode
#define C_OSL_DIG4 47  // DIG 4, anode
#define C_OSL_DIG5 21  // DIG 4, anode
#define C_OSL_DIG6 20  // DIG 4, anode
/* PCB */

/*
-- 数字とそのときの状態をビット表現 --
Num, G, F, E, D, C, B, A, bin表現(最上位は未使用なので0)
0  , 0, 1, 1, 1, 1, 1, 1, 0b00111111
1  , 0, 0, 0, 0, 1, 1, 0, 0b00000110
2  , 1, 0, 1, 1, 0, 1, 1, 0b01011011
3  , 1, 0, 0, 1, 1, 1, 1, 0b01001111
4  , 1, 1, 0, 0, 1, 1, 0, 0b01100110
5  , 1, 1, 0, 1, 1, 0, 1, 0b01101101
6  , 1, 1, 1, 1, 1, 0, 1, 0b01111101
7  , 0, 1, 0, 0, 1, 1, 1, 0b00100111
8  , 1, 1, 1, 1, 1, 1, 1, 0b01111111
9  , 1, 1, 0, 1, 1, 1, 1, 0b01101111
*/
/* !---------- 7Segment LED PIN MAP ----------! */

/* -- 点灯・消灯の判断 -- */
/*
Segment, 判断(1), 判断(2)
OSL_A, digits[n] & 0b00000001, digts[n] & (1 << 0)
OSL_B, digits[n] & 0b00000010, digts[n] & (1 << 1)
OSL_C, digits[n] & 0b00000100, digts[n] & (1 << 2)
OSL_D, digits[n] & 0b00001000, digts[n] & (1 << 3)
OSL_E, digits[n] & 0b00010000, digts[n] & (1 << 4)
OSL_F, digits[n] & 0b00100000, digts[n] & (1 << 5)
OSL_G, digits[n] & 0b01000000, digts[n] & (1 << 6)

このように使う
digitalWrite(segmentsPins[i], digits[n] & (1 << i) ? HIGH : LOW);
*/

typedef enum { A, B, C } SevenSegmentID;

extern void initLedDisplay();
extern void displayTime(const tm *timeInfo, const float *humi, const float *temp);

#endif