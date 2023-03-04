#include "gate_driver.h"
#include "io_driver.h"

static QueueHandle_t command_queue;
static driver_state_t state = BOOT_UP;
static gate_t *gate = NULL;
static gate_driver_setting_t *gate_driver_setting = NULL;
static gate_driver_t gate_driver_instance = {
    .num_gate = 0,
    .send_gate_command = NULL,
    .gate_driver_init = NULL,
    .gate_driver_get_state = NULL,
    .print_gate_driver_setting = NULL,
};

void send_gate_command(gate_command_t command)
{
    // Implementation to send gate command to hardware
    if (xQueueSend(command_queue, &command, 0) != pdTRUE)
    {
        printf("Failed to send command to queue.\n");
    }
}

driver_state_t gate_driver_get_state()
{
    return state;
}

void uninit_gate_driver()
{
    // Free the memory allocated for the gate array and gate settings
    if (gate != NULL)
    {
        free(gate);
        gate = NULL;
    }

    if (gate_driver_setting != NULL)
    {
        free(gate_driver_setting);
        gate_driver_setting = NULL;
    }
}

uint8_t gate_driver_init(gate_driver_setting_t *setting, int num_gates)
{
    // Reset the gate driver state and number of gates
    state = INIT;

    // Free the memory allocated for the gate array and gate settings
    uninit_gate_driver();

    // Allocate memory for the gate array and gate settings
    gate = calloc(num_gates, sizeof(gate_t));
    gate_driver_setting = calloc(num_gates, sizeof(gate_driver_setting_t));

    if (!gate || !gate_driver_setting)
    {
        // handle error
        return 0;
    }

    // Initialize all gates with default data
    memset(gate, 0, sizeof(gate_t) * num_gates);

    // Initialize all gate settings
    memcpy(gate_driver_setting, setting, sizeof(gate_driver_setting_t) * num_gates);

    // Initialize the gate driver state
    gate_driver_instance.num_gate = num_gates;
    state = READY;

    return 1;
}

void print_gate_driver_setting()
{
    printf("Gate settings:\n");
    for (int i = 0; i < gate_driver_instance.num_gate; i++)
    {
        printf("Gate %d:\n", i);
        printf("     Right gate settings:\n");
        printf("          is_use: %d\n", gate_driver_setting[i].right.is_use);
        printf("          Trigger pin: port %d, pin %d\n", gate_driver_setting[i].right.trigger.port, gate_driver_setting[i].right.trigger.pin);
        printf("          Open limit pin: port %d, pin %d\n", gate_driver_setting[i].right.open_limit.port, gate_driver_setting[i].right.open_limit.pin);
        printf("          Close limit pin: port %d, pin %d\n", gate_driver_setting[i].right.close_limit.port, gate_driver_setting[i].right.close_limit.pin);
        printf("          is_trigger: %d\n", gate[i].right.is_trigger);
        printf("          is_open: %d\n", gate[i].right.is_open);
        printf("          is_close: %d\n", gate[i].right.is_close);
        printf("          state: %d\n", gate[i].right.state);
        printf("     Left gate settings:\n");
        printf("          is_use: %d\n", gate_driver_setting[i].left.is_use);
        printf("          Trigger pin: port %d, pin %d\n", gate_driver_setting[i].left.trigger.port, gate_driver_setting[i].left.trigger.pin);
        printf("          Open limit pin: port %d, pin %d\n", gate_driver_setting[i].left.open_limit.port, gate_driver_setting[i].left.open_limit.pin);
        printf("          Close limit pin: port %d, pin %d\n", gate_driver_setting[i].left.close_limit.port, gate_driver_setting[i].left.close_limit.pin);
        printf("          is_trigger: %d\n", gate[i].left.is_trigger);
        printf("          is_open: %d\n", gate[i].left.is_open);
        printf("          is_close: %d\n", gate[i].left.is_close);
        printf("          state: %d\n", gate[i].left.state);
    }
}

gate_driver_t *get_gate_driver_instance()
{
    if (state == BOOT_UP)
    {
        // Initialize the gate driver instance
        gate_driver_instance.send_gate_command = send_gate_command;
        gate_driver_instance.gate_driver_init = gate_driver_init;
        gate_driver_instance.gate_driver_get_state = gate_driver_get_state;
        gate_driver_instance.print_gate_driver_setting = print_gate_driver_setting;
    }

    return &gate_driver_instance;
}

static void gate_controller_task(void *pvParameters)
{
    gate_command_t gate_command;
    output_command_t output_cmd;
    io_driver_t *io_drv_obj = get_io_driver_instance();

    int current_sensor_val = 0;
    int old_sensor_value = 0;

    while (1)
    {
        memset(&gate_command, 0, sizeof(gate_command));
        memset(&output_cmd, 0, sizeof(output_cmd));

        if (xQueueReceive(command_queue, &gate_command, 0) == pdTRUE)
        {
            // read current state of sensor pin
            old_sensor_value = io_drv_obj->io_read_pin_value(gate_command.position, gate_command.gate_no);

            // start motor util receive sensor signal
            output_cmd.pin = gate_command.gate_no;
            output_cmd.port = gate_command.position;
            output_cmd.state = gate_command.command_name;
            get_io_driver_instance()->command_enqueue(&output_cmd);

            // wait sensor pin is change when motor start run
            while (1)
            {
                current_sensor_val = io_drv_obj->io_read_pin_value(gate_command.position, gate_command.gate_no);
                if (old_sensor_value != current_sensor_val)
                {
                    old_sensor_value = current_sensor_val;
                    break;
                }
            }

            // wait sensor pin is trigger when motor
            while (1)
            {
                current_sensor_val = io_drv_obj->io_read_pin_value(gate_command.position, gate_command.gate_no);
                if (old_sensor_value != current_sensor_val)
                {
                    old_sensor_value = current_sensor_val;
                    break;
                }
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void gate_driver_start_task(void)
{
    gate_driver_setting_t gate_setting_array[2] = {
        {
            .right = {
             .is_use = true,
             .trigger = {.port = 1, .pin = 2},
             .open_limit = {.port = 3, .pin = 4},
             .close_limit = {.port = 5, .pin = 6},
            },
            .left = {
                .is_use = true,
                .trigger = {.port = 7, .pin = 8},
                .open_limit = {.port = 9, .pin = 10},
                .close_limit = {.port = 11, .pin = 12},
            }
        },
        {
            .right = {
                .is_use = true,
                .trigger = {.port = 13, .pin = 14},
                .open_limit = {.port = 15, .pin = 16},
                .close_limit = {.port = 17, .pin = 18},
            },
            .left = {
                .is_use = true,
                .trigger = {.port = 19, .pin = 20},
                .open_limit = {.port = 21, .pin = 22},
                .close_limit = {.port = 23, .pin = 24},
            },
        },
    };
    gate_driver_t *gate_driver = get_gate_driver_instance();
    gate_driver->gate_driver_init(gate_setting_array, 2);
    gate_driver->print_gate_driver_setting();

    xTaskCreate(gate_controller_task, "gate_controller_task", 4096, NULL, 5, NULL);
}