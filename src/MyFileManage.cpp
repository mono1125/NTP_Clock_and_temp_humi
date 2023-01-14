#include "MyFileManage.h"

static char TAG[] = "MyFileManage";

void initMyFileManage() {
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    ESP_LOGE(TAG, "LittleFS Mount Failed");
    return;
  }
  static char buf[1024];
  listDir(LittleFS, "/", 2);
}

int getJsonObj(const char *path, JsonDocument &doc) {
  static char buf[2048];
  if (readFile(LittleFS, path, buf, sizeof(buf)) != 0) {
    ESP_LOGE(TAG, "readFile Error");
    return -1;
  }
  if (myDeserializeJson(doc, buf) != 0) {
    return -1;
  }
  return 0;
}

int writeJsonFile(const char *path, JsonDocument &doc) {
  static char buf[2048];

  serializeJson(doc, buf);
  if (writeFile(LittleFS, path, buf) != 0) {
    return -1;
  };
  return 0;
}

static void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  ESP_LOGI(TAG, "Listing directory: %s", dirname);

  File root = fs.open(dirname);
  if (!root) {
    ESP_LOGE(TAG, "failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    ESP_LOGE(TAG, "not a directory");
    root.close();
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      ESP_LOGI(TAG, "DIR: %s", file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      ESP_LOGI(TAG, "FILE: %s, SIZE: %d", file.name(), file.size());
    }
    file = root.openNextFile();
  }
  file.close();
}

static void printFile(fs::FS &fs, const char *path) {
  static char buf[1024];
  if (fs.exists(path)) {
    readFile(LittleFS, path, buf, sizeof(buf));
    ESP_LOGI(TAG, "READ FILE (%s): %s", path, buf);
  }
}

static int readFile(fs::FS &fs, const char *path, char *buf, const size_t buf_len) {
  static unsigned int i = 0;

  ESP_LOGI(TAG, "Reading file: %s, Buffer size: %d", path, buf_len);

  File file = fs.open(path);
  if (!file) {
    ESP_LOGE(TAG, "failed to open");
    return -1;
  }
  if (file.isDirectory()) {
    ESP_LOGE(TAG, "failed to open");
    file.close();
    return -1;
  }
  if (file.size() > buf_len) {
    ESP_LOGE(TAG, "File Size too large > buf");
    file.close();
    return -1;
  }
  while (file.available() && (i < buf_len)) {
    buf[i] = file.read();
    i++;
  }
  file.close();

  return 0;
}

static int writeFile(fs::FS &fs, const char *path, const char *data) {
  static int return_code = 0;
  ESP_LOGI(TAG, "Writing file: %s", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    ESP_LOGE(TAG, "failed to open file for writing");
    return -1;
  }
  if (file.print(data)) {
    ESP_LOGI(TAG, "file written");
  } else {
    ESP_LOGE(TAG, "write failed");
    return_code = -1;
  }
  file.close();

  return return_code;
}

int myDeserializeJson(JsonDocument &doc, char *data) {
  static int return_code = 0;

  DeserializationError err = deserializeJson(doc, data);
  switch (err.code()) {
    case DeserializationError::Ok:
      /* deserialize success */
      ESP_LOGI(TAG, "Deserialization Success");
      break;
    case DeserializationError::EmptyInput:
      ESP_LOGE(TAG, "Deserialization Error: Empty Input");
      return_code = -1;
      break;
    case DeserializationError::IncompleteInput:
      ESP_LOGE(TAG, "Deserialization Error: Incomplete Input");
      return_code = -1;
      break;
    case DeserializationError::InvalidInput:
      ESP_LOGE(TAG, "Deserialization Error: Invalid Input");
      return_code = -1;
      break;
    case DeserializationError::NoMemory:
      ESP_LOGE(TAG, "Deserialization Error: No Memory");
      return_code = -1;
      break;
    default:
      ESP_LOGE(TAG, "Deserialization failed");
      return_code = -1;
      break;
  }

  return return_code;
}