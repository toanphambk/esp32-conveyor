#pragma once

#include <stdint.h>

#define ENCODER_A_PORT  0
#define ENCODER_A_PIN   1
#define ENCODER_B_PORT  0
#define ENCODER_B_PIN   2

#define ENCODER_NUM_PULSE   360

typedef enum {
    CLOCKWISE_DIR = 0,
    COUNTER_CLOCKWISE,
} encoder_dir_t;

typedef struct {
    encoder_dir_t direction;
    uint32_t pulse_count;
    uint32_t position; /* 360 degrees */
} encoder_t;