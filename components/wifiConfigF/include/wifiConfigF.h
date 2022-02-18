#ifndef _WIFCONFIGF_H_
#define _WIFCONFIGF_H_
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"

#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "utils.h"
#include "mqtt_client.h"

#ifdef __cplusplus
extern "C"
{
#endif
  static EventGroupHandle_t s_wifi_event_group;
  typedef void (*handleFunc)(void);
  // typedef uint8_t (*mqttConnected)(uint16_t);
  typedef struct
  {
    handleFunc wifiStart;
    handleFunc routerGotIp;
    handleFunc routerDisconnected;
    handleFunc firstHandler;
    handleFunc internetConnected;
    handleFunc internetDisconnected;
    handleFunc credentialCB;
    // mqttConnected isMqttConnected;
    // uint8_t *network;
  } handle_args_t;

  void initSmartConfig(handle_args_t *args);
#ifdef __cplusplus
}
#endif
#endif