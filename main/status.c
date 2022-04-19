#include "status.h"
#include "esp_log.h"

#define STATUS_TAG "STATUS"
void ledIndicator(void *params)
{
    datasend_t rx_status;

    while(1)
    {
        xQueueReceive(queue_led, &rx_status , 100/portTICK_PERIOD_MS);
        
        ESP_LOGI(STATUS_TAG, "QUEUE RECEIVED");
        switch (rx_status.color)
        {
        case 1:
            ESP_LOGI(STATUS_TAG, "QUEUE RECEIVED CONNECT");
            gpio_set_level(RED, 0);
            gpio_set_level(GREEN, 0);
            gpio_set_level(BLUE, 1);
            vTaskDelay(portMAX_DELAY);
            // xQueueReset(queue_led);
        break;

        case 2:
        case 3:
            ESP_LOGI(STATUS_TAG, "QUEUE RECEIVED ERROR");
                gpio_set_level(RED, 1);
                gpio_set_level(GREEN, 0);
                gpio_set_level(BLUE, 0);
                vTaskDelay((1000 / portTICK_PERIOD_MS));
                gpio_set_level(RED, 0);
                gpio_set_level(GREEN, 0);
                gpio_set_level(BLUE, 0);
            // xQueueReset(queue_led);
        break;

        case 4:
            ESP_LOGI(STATUS_TAG, "QUEUE RECEIVED ERROR");
                gpio_set_level(RED, 1);
                gpio_set_level(GREEN, 0);
                gpio_set_level(BLUE, 0);
                vTaskDelay((1000 / portTICK_PERIOD_MS));
                gpio_set_level(RED, 0);
                gpio_set_level(GREEN, 0);
                gpio_set_level(BLUE, 0);
            // xQueueReset(queue_led);
        break;

        default:
            ESP_LOGI(STATUS_TAG, "WAITING CONNECTION");
            gpio_set_level(RED, 0);
            gpio_set_level(GREEN, 1);
            gpio_set_level(BLUE, 0);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        break;

        }
    }
    
}

void initLed(void){

    queue_led = xQueueCreate( 10 , sizeof(datasend_t));
    xTaskCreate(ledIndicator, "Led Status", 2048, NULL, configMAX_PRIORITIES - 5, NULL);
        
}