#include "status.h"
#include "esp_log.h"

#define STATUS_TAG "STATUS"
#define TICKS 100/portTICK_PERIOD_MS

void ledIndicator(void *params)
{
    datasend_t rx_status;

    while(1)
    {
        xQueueReceive(queue_led, &rx_status , 100/portTICK_PERIOD_MS);
        ESP_LOGI(STATUS_TAG, "QUEUE RECEIVED");

        switch (rx_status.color)
        {
        case D_BLUE:
            ESP_LOGI(STATUS_TAG, "QUEUE RECEIVED CONNECT");
            gpio_set_level(RED, 0);
            gpio_set_level(GREEN, 0);
            gpio_set_level(BLUE, 1);
            vTaskDelay(TICKS* rx_status.ton);
            gpio_set_level(RED, 0);
            gpio_set_level(GREEN, 0);
            gpio_set_level(BLUE, 1);
            vTaskDelay(TICKS* (rx_status.toff-1));
        break;

        case D_GREEN:
            ESP_LOGI(STATUS_TAG, "MQTT DESCONNECT");
                gpio_set_level(RED, 0);
                gpio_set_level(GREEN, 1);
                gpio_set_level(BLUE, 0);
                vTaskDelay(TICKS*rx_status.ton);
                gpio_set_level(RED, 0);
                gpio_set_level(GREEN, 0);
                gpio_set_level(BLUE, 0);
                vTaskDelay(TICKS*(rx_status.toff-1));

        break;

            case D_RED:
            ESP_LOGI(STATUS_TAG, "QUEUE RECEIVED ERROR");
                gpio_set_level(RED, 1);
                gpio_set_level(GREEN, 0);
                gpio_set_level(BLUE, 0);
                vTaskDelay(TICKS*rx_status.ton);
                gpio_set_level(RED, 0);
                gpio_set_level(GREEN, 0);
                gpio_set_level(BLUE, 0);
                vTaskDelay(TICKS*(rx_status.toff-1));
        break;

        default:
            ESP_LOGI(STATUS_TAG, "WAITING CONNECTION");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
                gpio_set_level(RED, 0);
                gpio_set_level(GREEN, 1);
                gpio_set_level(BLUE, 0);
                vTaskDelay(TICKS*5);
                gpio_set_level(RED, 0);
                gpio_set_level(GREEN, 0);
                gpio_set_level(BLUE, 0);
                vTaskDelay(TICKS*4);
        break;

        }
    }
    
}

void initLed(void){

    queue_led = xQueueCreate( 10 , sizeof(datasend_t));
    xTaskCreate(ledIndicator, "Led Status", 2048, NULL, configMAX_PRIORITIES - 5, NULL);
        
}