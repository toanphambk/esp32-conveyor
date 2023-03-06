#pragma once
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include "tcp_client.h"
#include "string.h"

#define EXAMPLE_PCNT_HIGH_LIMIT 4000
#define EXAMPLE_PCNT_LOW_LIMIT  -2000
#define EXAMPLE_EC11_GPIO_A 15
#define EXAMPLE_EC11_GPIO_B 16

void encoder_start(void);
int get_pulse();