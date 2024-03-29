#include "mqtt.h"
#include "status.h"
#define RED (gpio_num_t)14
#define GREEN (gpio_num_t)12
#define BLUE (gpio_num_t)27
datasend_t status;

void mqttConfigGPRS(void)
{

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://plataforma.connecton.com.br",
        .port = 1883,
        .lwt_qos = 2,
        .lwt_retain = 1,
        .event_handle = mqttEventHandler,
    };
    mqttGPRS = esp_mqtt_client_init(&mqtt_cfg);
    
    esp_mqtt_client_start(mqttGPRS);
}

void mqttStart(void)
{
    esp_mqtt_client_start(mqttGPRS);
}

esp_err_t mqttEventHandler(esp_mqtt_event_handle_t event)
{
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
       
        status.color = 3;
        status.ton = 10;
        status.toff = 10;

        xQueueSend(queue_led, &status, 0);

        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");

        break;

    case MQTT_EVENT_DISCONNECTED:

        status.color = 1;
        status.ton = 10;
        status.toff = 10;

        xQueueSend(queue_led, &status, 0);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DATA");
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(MQTT_TAG, "MQTT_EVENT_ERROR");
        status.color = 2;
        status.ton = 10;
        status.toff = 10;
        ESP_LOGW(MQTT_TAG, "Empty Operator - Error\n");
        xQueueSend(queue_led, &status, 0);
        break;

    default:
        ESP_LOGI(MQTT_TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}
void mqttStop(void)
{
    if (mqttGPRS != NULL)
    {
        esp_mqtt_client_stop(mqttGPRS);
    }
}