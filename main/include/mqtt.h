#ifndef _MQTT_
#define _MQTT_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "esp_spi_flash.h"
#include "internet.h"
#include "FileManager.h"
#include "utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MQTT_TAG "MQTT_LIB"
#define LED_CON 2

    typedef struct
    {
        char topic[40];
        char msg[400];
    } rx_event_t;

    extern const uint8_t client_cert_pem_start[] asm("_binary_ca_crt_start");
    extern const uint8_t client_cert_pem_end[] asm("_binary_ca_crt_end");

    static const int CONNECTED_BIT = BIT0;
    static EventGroupHandle_t mqttEvent;
    static esp_mqtt_client_handle_t mqttClient;
    static esp_mqtt_client_handle_t mqttGPRS;
    xQueueHandle rxQueue;

    static char macGlobal[20];

    void mqttSetup();
    void mqttConfigGPRS(void);
    void mqttConfig(void);
    void mqttStart(void);
    void mqttStop(void);
    void mqttDestroy(void);
    esp_err_t mqttEventHandler(esp_mqtt_event_handle_t event);
    int mqttPub(const char *topic, const char *data, int qos);
    uint8_t mqttIsConnected(uint16_t timeout);
    void mqttSub(const char *topic);
    void subscribeOnConnect(void);
    void lwtConnected(void);

#ifdef __cplusplus
}
#endif

#endif