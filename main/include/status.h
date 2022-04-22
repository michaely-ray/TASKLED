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

#define D_GREEN 1
#define D_RED 2
#define D_BLUE 3

// #define STATUS_WAIT {.color=1, ton =10, toff=10 }

QueueHandle_t queue_led;
    
typedef struct{
    uint8_t color;
    uint8_t ton;
    uint8_t toff;
}datasend_t;

datasend_t s_status;
void ledIndicator(void *params);
void initLed();

#endif
