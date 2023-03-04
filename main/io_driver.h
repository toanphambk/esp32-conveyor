#ifndef IO_DRIVER_H
#define IO_DRIVER_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "driver/i2c.h"
#include "global_header.h"

#define IN_PORT_NUM 2
#define OUT_PORT_NUM 2
#define SCAN_INTERVAL 50

#define IN_PORT_NUM 2
#define OUT_PORT_NUM 2

#define PCF8574_OUT_PORT_0 0b0100011 // I2C address of the PCF8574 U1
#define PCF8574_OUT_PORT_1 0b0100111 // I2C address of the PCF8574 U39

#define PCF8574_IN_PORT_0 0b0100010 // I2C address of the PCF8574 U2
#define PCF8574_IN_PORT_1 0b0100000 // I2C address of the PCF8574 U40

typedef struct
{
    uint8_t input[IN_PORT_NUM];
    uint8_t output[OUT_PORT_NUM];
} port_address_t;

/**
 *                    0     1     2     3     4     5     6     7   (gate no)
 * right gate[0] |----*-----*-----*-----*-----*-----*-----*-----*|  
 *                           /
 *                    \ 
 * left gate[1]  |----*-----*-----*-----*-----*-----*-----*-----*|  
 *                    0     1     2     3     4     5     6     7   (gate no)
*/
typedef struct
{
    uint8_t input[IN_PORT_NUM][8];
    uint8_t output[OUT_PORT_NUM][8];
} io_data_t;

typedef struct
{
    uint8_t port;
    uint8_t pin;
    bool state;
} output_command_t;

/* Structure to hold gate driver instance */
typedef struct
{
    void (*io_scan_task)(void *pvParameter);
    void (*get_io_data)(io_data_t *buf);
    void (*print_port_data)(io_data_t buff);
    void (*command_enqueue)(output_command_t *command);
    void (*io_test)(void);
    uint8_t (*io_read_pin_value)(uint8_t port, uint8_t pin);
    driver_state_t (*io_driver_get_state)(void);
} io_driver_t;

/* Function to get an instance of the gate driver */
io_driver_t *get_io_driver_instance(void);
void io_scan_task_start(void);
#endif