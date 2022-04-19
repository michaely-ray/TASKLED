#ifndef _STATUS_H_
#define _STATUS_H_

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define RED (gpio_num_t)14
#define GREEN (gpio_num_t)12
#define BLUE (gpio_num_t)27

QueueHandle_t queue_led;
    
typedef struct{
    uint8_t color;
    uint8_t ton;
    int toff;
}datasend_t;

datasend_t s_status;
void ledIndicator(void *params);
void initLed();

#endif
