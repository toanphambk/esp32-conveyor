#ifndef GATE_DRIVER_H
#define GATE_DRIVER_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "freertos/queue.h"
#include <string.h>
#include "global_header.h"
/* Enumeration for gate position */
typedef enum
{
    RIGHT_GATE = 0,
    LEFT_GATE
} gate_position_t;

/* Enumeration for gate command name */
typedef enum
{
    GATE_STOP_COMMAND = 0,
    GATE_TRIGGER_COMMAND,
    GATE_CLOSE_COMMAND,
    GATE_OPEN_COMMAND,
} gate_command_name_t;

/* Enumeration for gate state */
typedef enum
{
    GATE_READY = 0,
    GATE_CLOSED,
    GATE_MOVING,
    GATE_OPEN,
} gate_state_t;

/* Structure to hold gate command */
typedef struct
{
    uint8_t gate_no;
    gate_command_name_t command_name;
    gate_position_t position;
} gate_command_t;

/* Structure to hold gate data */
typedef struct
{
    bool is_trigger;
    bool is_open;
    bool is_close;
    gate_state_t state;
} gate_data_t;

/* Structure to hold pin address */
typedef struct
{
    uint8_t port;
    uint8_t pin;
} pin_address_t;

/* Structure to hold gate settings */
typedef struct
{
    bool is_use;
    pin_address_t trigger;
    pin_address_t open_limit;
    pin_address_t close_limit;
} gate_setting_t;

/* Structure to hold gate driver settings */
typedef struct
{
    gate_setting_t right;
    gate_setting_t left;
} gate_driver_setting_t;

/* Structure to hold gate driver gate data */
typedef struct
{
    gate_data_t right;
    gate_data_t left;
} gate_t;

/* Structure to hold gate driver instance */
typedef struct
{
    uint8_t num_gate;
    void (*send_gate_command)(gate_command_t);
    uint8_t (*gate_driver_init)(gate_driver_setting_t *gate_driver_settings, int num_gates);
    driver_state_t (*gate_driver_get_state)(void);
    void (*print_gate_driver_setting)();
} gate_driver_t;

/* Function to get an instance of the gate driver */
gate_driver_t *get_gate_driver_instance(void);
void gate_driver_start_task(void);
#endif
