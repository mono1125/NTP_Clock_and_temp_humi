#ifndef MY_TCP_H
#define MY_TCP_H

#include "MyConfig.h"
#include "MyWiFi.h"

#define TCP_MSG_SIZE (512)

extern volatile QueueHandle_t tcpQueue;

extern void initTCPQueue();
extern void TCPTask(void *pvParameters);

#endif