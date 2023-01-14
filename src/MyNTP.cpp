#include "MyNtp.h"

static const char TAG[] = "MyNTP";

static void sntpCallback(struct timeval *tv) {
  ESP_LOGI(TAG, "SNTP Sync");
}

void beginNtp(uint32_t interval_ms) {
  sntp_set_sync_interval(interval_ms);  // 1min
  sntp_set_time_sync_notification_cb(sntpCallback);
  configTzTime("JST-9", "ntp.aws.com", "ntp.nict.jp", "time.google.com");
}