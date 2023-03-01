#include "gate_driver.h"

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
