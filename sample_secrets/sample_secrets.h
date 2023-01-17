#ifndef SAMPLE_SECRETS_H
#define SAMPLE_SECRETS_H

/* Wi-Fi */
#define WIFI_SSID "Your WiFi SSID"
#define WIFI_PASS "Your WiFi PASS"

#define CONF_MODE_IP   "192.168.2.254"
#define CONF_AP_SSID   "myesp32"
#define CONF_AP_PASS   "myesp32-pass"
#define CONF_AP_SUBNET "255.255.255.0"

/* Web Server */
#define HTTP_PORT 80
#define HTTP_USER "admin"
#define HTTP_PASS "passw0rd"

/* MQTT */
#define THING_NAME    "Your Thing (IoT Core)"
#define MQTT_ENDPOINT "xxxxxxxxxxxxxx.iot.ap-northeast-1.amazonaws.com"
#define MQTT_PORT     8883

/* MQTT Topic */
extern const char TEST_PUB_TOPIC[];
extern const char PROD_PUB_TOPIC[];
extern const char DEVICE_CPU_TEMP_PUB_TOPIC[];
extern const char DEVICE_FREE_HEAP_PUB_TOPIC[];
extern const char DEVICE_RESPONSE_PUB_TOPIC[];
extern const char CONF_SUB_TOPIC[];

/* Amazon ROOT CA */
extern const char ROOT_CA[];
/* Device Cert */
extern const char CLIENT_CERT[];
/* Device PrivateKey */
extern const char PRIVATE_KEY[];

#endif
