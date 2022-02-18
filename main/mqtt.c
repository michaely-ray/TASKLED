
#include "driver/gpio.h"
#include "internet.h"
#include "mqtt.h"
#define RED (gpio_num_t)14
#define GREEN (gpio_num_t)12
#define BLUE (gpio_num_t)27
uint8_t errors;

void mqttSetup()
{

    gpio_pad_select_gpio(LED_CON);
    gpio_set_direction(LED_CON, GPIO_MODE_OUTPUT);
    rxQueue = xQueueCreate(10, sizeof(rx_event_t));
    mqttEvent = xEventGroupCreate();
    xEventGroupClearBits(mqttEvent, CONNECTED_BIT);
}
// esp_mqtt_client_handle_t mqttConnect(void)
void mqttConfig(void)
{
    char lwtMsg[50];
    uint8_t macAddr[6];
    esp_efuse_mac_get_default(macAddr);
    sprintf(macGlobal, "" MACSTR "", MAC2STR(macAddr));
    sprintf(lwtMsg, "{\"mac\":\"%s\"}", macGlobal);

    esp_mqtt_client_config_t mqtt_cfg = {
        //.uri = "mqtts://plataforma.connecton.com.br",
        .uri = "mqtt://plataforma.connecton.com.br",
        //.cert_pem = (const char *)client_cert_pem_start,
        //.port = 8883,
        .port = 1883,
        //.username = "blueraven",
        //.password = "blueravenlocal",
        .lwt_topic = "/lwt/desconectado",
        .lwt_msg = lwtMsg,
        .lwt_msg_len = strlen(lwtMsg),
        .lwt_qos = 2,
        .lwt_retain = 1,
        .event_handle = mqttEventHandler,
    };
    mqttClient = esp_mqtt_client_init(&mqtt_cfg);
    //     return mqttClient;
}

void mqttConfigGPRS(void)
{
    char lwtMsg[50];
    uint8_t macAddr[6];
    esp_efuse_mac_get_default(macAddr);
    sprintf(macGlobal, "" MACSTR "", MAC2STR(macAddr));
    sprintf(lwtMsg, "{\"mac\":\"%s\"}", macGlobal);

    esp_mqtt_client_config_t mqtt_cfg = {
        //.uri = "mqtts://plataforma.connecton.com.br",
        .uri = "mqtt://plataforma.connecton.com.br",
        //.cert_pem = (const char *)client_cert_pem_start,
        //.port = 8883,
        .port = 1883,
        //.username = "blueraven",
        //.password = "blueravenlocal",
        .lwt_topic = "/lwt/desconectado",
        .lwt_msg = lwtMsg,
        .lwt_msg_len = strlen(lwtMsg),
        .lwt_qos = 2,
        .lwt_retain = 1,
        .event_handle = mqttEventHandler,
    };
    mqttGPRS = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(mqttGPRS);
    //     return mqttClient;
}

void mqttStart(void)
{
    esp_mqtt_client_start(mqttClient);
}
void mqttDestroy(void)
{
    esp_mqtt_client_destroy(mqttClient);
}
void mqttStop(void)
{
    if (mqttClient != NULL)
    {

        esp_mqtt_client_stop(mqttClient);
    }
}
esp_err_t mqttEventHandler(esp_mqtt_event_handle_t event)
{
        gpio_pad_select_gpio(RED);
        gpio_pad_select_gpio(GREEN);
        gpio_pad_select_gpio(BLUE);
        gpio_set_direction(RED, GPIO_MODE_INPUT_OUTPUT);
        gpio_set_direction(GREEN, GPIO_MODE_INPUT_OUTPUT);
        gpio_set_direction(BLUE, GPIO_MODE_INPUT_OUTPUT);
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        errors = 0;
        char signUpStatus[5];
        gpio_set_level(LED_CON, 1);
        xEventGroupSetBits(mqttEvent, CONNECTED_BIT);
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
        nvs_str_read(signUpStatus, sizeof(signUpStatus), "SIGNUP");
        if (strcmp(signUpStatus, "1") == 0)
        {
            char *busJSON = (char *)calloc(512, 1);
            if (getBusData(&busJSON, 512) > 0)
            {
                char *sendMsg = (char *)calloc(1000, 1);
                char info[500];
                getInternetInfo(info);
                sprintf(sendMsg, "{\"info\":{%s},\"bus\":%s}", info, busJSON);
                ESP_LOGI(MQTT_TAG, "PUBLISH SIGNUP ");
                mqttPublish("/signup", sendMsg, 2);
                createSignupFile(0);
                nvs_str_save("SIGNUP", "0");
                free(sendMsg);
            }

            free(busJSON);
        }
        else
        {
            ESP_LOGI(MQTT_TAG, "ERRROOOR PUBLISH SIGNUP ");
        }
        subscribeOnConnect();
        lwtConnected();
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
        xEventGroupClearBits(mqttEvent, CONNECTED_BIT);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        while(1){
            
                gpio_set_level(RED, 0);
                gpio_set_level(GREEN, 0);
                gpio_set_level(BLUE, 1);
                vTaskDelay(pdMS_TO_TICKS(5000));
                int stop = 0;
                while(1){
                    stop++;
                }
        }
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        for(;;){
            
                gpio_set_level(RED, 0);
                gpio_set_level(GREEN, 0);
                gpio_set_level(BLUE, 1);
                vTaskDelay(pdMS_TO_TICKS(5000));
                int stop = 0;
                while(1){
                    stop++;
                }
        }
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DATA");
        rx_event_t evt;

        strncpy(evt.topic, event->topic, event->topic_len);
        strncpy(evt.msg, event->data, event->data_len);
        evt.topic[event->topic_len] = 0;
        evt.msg[event->data_len] = 0;
        // printf("topic: %s message: %s", evt.topic, evt.msg);
        xQueueSend(rxQueue, &evt, (TickType_t)0);
        break;

    case MQTT_EVENT_ERROR:
        errors++;
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_ERROR");
        if (errors >= 10)
        {
            esp_restart();
        }

        // esp_restart();
        // esp_mqtt_client_reconnect(mqttClient);
        break;

    default:
        ESP_LOGI(MQTT_TAG, "Other event id:%d", event->event_id);
        break;
    }
    networkType = gprs;
    return ESP_OK;
}

int mqttPub(const char *topic, const char *data, int qos)
{
    return esp_mqtt_client_publish(mqttClient, topic, data, 0, qos, 0);
}

uint8_t mqttIsConnected(uint16_t timeout)
{

    EventBits_t uxBits = xEventGroupWaitBits(mqttEvent, CONNECTED_BIT, false, true, pdMS_TO_TICKS(timeout));
    if ((uxBits & CONNECTED_BIT) == 0)
        return 0;
    else
    {
        return 1;
    }
}

void mqttSub(const char *topic)
{
    esp_mqtt_client_subscribe(mqttClient, topic, 0);
}

void subscribeOnConnect(void)
{
    char city[50];
    char topic[60];

    if (getCity(city) == ESP_OK)
    {
        sprintf(topic, "/daytime/%s", city);
        esp_mqtt_client_subscribe(mqttClient, topic, 0);
    }

    sprintf(topic, "/status/%s", macGlobal);
    esp_mqtt_client_subscribe(mqttClient, topic, 0);
    sprintf(topic, "/city/%s", macGlobal);
    esp_mqtt_client_subscribe(mqttClient, topic, 0);
    sprintf(topic, "/ota/%s", macGlobal);
    esp_mqtt_client_subscribe(mqttClient, topic, 0);
    sprintf(topic, "/command/%s", macGlobal);
    esp_mqtt_client_subscribe(mqttClient, topic, 0);
    sprintf(topic, "/signup/%s", macGlobal);
    esp_mqtt_client_subscribe(mqttClient, topic, 0);
    sprintf(topic, "/data/%s", macGlobal);
    esp_mqtt_client_subscribe(mqttClient, topic, 0);
    sprintf(topic, "/associate/%s", macGlobal);
    esp_mqtt_client_subscribe(mqttClient, topic, 0);
    sprintf(topic, "/getbus/%s", macGlobal);
    esp_mqtt_client_subscribe(mqttClient, topic, 0);
    sprintf(topic, "/erase/%s", macGlobal);
    esp_mqtt_client_subscribe(mqttClient, topic, 0);
    sprintf(topic, "/credential/%s", macGlobal);
    esp_mqtt_client_subscribe(mqttClient, topic, 0);
    sprintf(topic, "/setbus/%s", macGlobal);
    esp_mqtt_client_subscribe(mqttClient, topic, 0);

    esp_mqtt_client_subscribe(mqttClient, "/ota/blueraven", 0);
}

void lwtConnected(void)
{
    esp_chip_info_t espInfo;
    esp_chip_info(&espInfo);

    char lwtMsg[100];
    char format[] = "{\"mac\":\"%s\",\"reset\":%d,\"model\":%d,\"idf\":\"%s\",\"flash\":%d,\"product\":\"%s\"}";
    sprintf(lwtMsg, format, macGlobal, esp_reset_reason(), espInfo.model, esp_get_idf_version(), spi_flash_get_chip_size() / 1048576, CONFIG_PRODUCT_NAME);
    esp_mqtt_client_publish(mqttClient, "/lwt/conectado", lwtMsg, 0, 2, 1);
}