#ifndef _OTA_
#define _OTA_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "string.h"

#define otaTAG "OTA_LIB"
#define UPDATE_JSON_URL		"https://plataforma.connecton.com.br:8000/ota"

#ifdef __cplusplus
extern "C" {
#endif

static char rcv_buffer[200];
static uint8_t otaFlag = 0;

void otaSetup();
void checkUpdateTask(void *pvParameter);
bool otaCheckVersion();
void otaUpdate();
void otaCheck();
esp_err_t _httpEventHandler(esp_http_client_event_t *evt);
uint8_t getOtaFlag();
bool compareVersion(uint major, uint minor, uint patch);

#ifdef __cplusplus
}
#endif

#endif