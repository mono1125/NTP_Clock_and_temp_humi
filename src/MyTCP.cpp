#include "MyTCP.h"

static char TAG[] = "MyTCP";

volatile QueueHandle_t tcpQueue;

static WiFiClient client;

void initTCPQueue() {
  tcpQueue = xQueueCreate(3, sizeof(char) * TCP_MSG_SIZE);
  if (tcpQueue == NULL) {
    ESP_LOGE(TAG, "キューの作成に失敗しました");
    delay(3000);
    ESP.restart();
  }
}

void TCPTask(void *pvParameters) {
  Config *conf = (Config *)pvParameters;
  char   *buf;
  buf = (char *)malloc(sizeof(char) * TCP_MSG_SIZE);
  if (buf == NULL) {
    ESP_LOGE(TAG, "メモリ確保に失敗");
    delay(3000);
    ESP.restart();
  }
  while (1) {
    if (xQueueReceive(tcpQueue, buf, 0) == pdPASS) {
      client.connect(conf->targetIPAddress, conf->targetPort);
      for (int i = 0; i < TCP_MSG_SIZE; i++) {
        client.write(buf[i]);
      }
      client.stop();
      memset(buf, '\0', TCP_MSG_SIZE);
    }
    delay(200);
  }
  free(buf);
  vTaskDelete(NULL);
}