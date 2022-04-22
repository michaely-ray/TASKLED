#ifndef _MQTT_
#define _MQTT_

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "mqtt_client.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MQTT_TAG "MQTT_LIB"

    static esp_mqtt_client_handle_t mqttGPRS;

    void mqttConfigGPRS(void);
    void mqttStart(void);
    esp_err_t mqttEventHandler(esp_mqtt_event_handle_t event);
    void mqttStop(void);

#ifdef __cplusplus
}
#endif

#endif