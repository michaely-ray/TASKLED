
#include "esp_log.h"
#include "driver/gpio.h"
#include "pppos_client.h"

#define RED (gpio_num_t)14
#define GREEN (gpio_num_t)12
#define BLUE (gpio_num_t)27

void app_main()
{
    gpio_pad_select_gpio(RED);
    gpio_pad_select_gpio(GREEN);
    gpio_pad_select_gpio(BLUE);
    gpio_set_direction(RED, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(GREEN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(BLUE, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(GREEN, 1);
    gpio_set_level(RED, 0);
    gpio_set_level(BLUE, 0);

    ppposStart();
}
