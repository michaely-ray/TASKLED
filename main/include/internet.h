#ifndef _INTERNET_
#define _INTERNET_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include <esp_http_server.h>
#include "mdns.h"

#include "FileManager.h"
#include "paramDefinition.h"
#include "ota.h"
#include "mqtt.h"
#include "endpoints.h"
#include "pppos_client.h"
#include "wifiConfigF.h"
#include <sys/param.h>
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sys.h"

typedef struct
{
  char ssid[32];
  char password[64];
} wifiParam_t;

#ifdef __cplusplus
extern "C"
{
#endif

  static uint8_t apIsRaised = 0;
  extern uint8_t networkType;
  extern uint8_t credentialStatus;
  static bool scanDone = false;

  static esp_netif_t *wifiAP = NULL;

  TaskHandle_t xcheckConnectionHandler;

  static const char *INTERNET_TAG = "INTERNET";

  void internetInit();
  void wifiManagerInit();
  static void checkConnectionTask(void *pvParameter);
  void raiseAP();
  void dropAP();
  uint8_t connect(char *ssid, char *password);
  static httpd_handle_t startWebserver();
  uint16_t scanNetworks(uint16_t number, wifi_ap_record_t *ap_info);
  static void eventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
  static void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
  int mqttPublish(const char *topic, const char *data, int qos);
  uint8_t isConnected(uint16_t timeout);
  void forceOta();
  uint8_t isOta();
  void getInternetInfo(char *info);

#ifdef __cplusplus
}
#endif

#endif