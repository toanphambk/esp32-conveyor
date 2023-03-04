#include "io_driver.h"

static io_data_t io_data;
static driver_state_t state = BOOT_UP;
static port_address_t port_adress = {
    .output = {PCF8574_OUT_PORT_0, PCF8574_OUT_PORT_1},
    .input = {PCF8574_IN_PORT_0, PCF8574_IN_PORT_1},
};

static io_driver_t io_driver_instance = {
    .io_driver_get_state = NULL,
    .get_io_data = NULL,
    .print_port_data = NULL,
    .command_enqueue = NULL,
    .io_scan_task = NULL,
    .io_test = NULL,
};

static QueueHandle_t command_queue;

driver_state_t io_driver_get_state()
{
    return state;
}

void get_io_data(io_data_t *buf)
{
    for (uint8_t port = 0; port < IN_PORT_NUM; port++)
    {
        for (int pin = 0; pin < 8; pin++)
        {
            buf->input[port][pin] = io_data.input[port][pin];
        }
    }
    
    for (uint8_t port = 0; port < OUT_PORT_NUM; port++)
    {
        for (int pin = 0; pin < 8; pin++)
        {
            buf->output[port][pin] = io_data.output[port][pin];
        }
    }
}

esp_err_t pcf8574_write_output(output_command_t command)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, port_adress.output[command.port] << 1 | I2C_MASTER_WRITE, true);
    uint8_t data = 0xff;
    // bit to byte data
    for (int pin = 0; pin < 8; pin++)
    {
        if (io_data.output[command.port][pin])
        {
            data &= ~(1 << pin);
        }
    }
    data &= ~(1 << command.pin);
    data |= (!command.state << command.pin);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        printf("Port Data write error\n");
    }
    // update to port data
    io_data.output[command.port][command.pin] = command.state;
    return ret;
}

esp_err_t pcf8574_read_port(uint8_t port)
{
    uint8_t read_value = 0xff;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, port_adress.input[port] << 1 | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &read_value, I2C_MASTER_ACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        printf("Port Data read error\n");
    }
    for (int pin = 0; pin < 8; pin++)
    {
        io_data.input[port][pin] = (read_value & (1 << pin)) == 0;
    }
    return ret;
}

esp_err_t init_i2c()
{
    state = INIT;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 32,
        .scl_io_num = 33,
        .master.clk_speed = 100000,
    };
    esp_err_t ret = i2c_param_config(I2C_NUM_1, &conf);
    if (ret != ESP_OK)
    {
        printf("Error when config i2c\n");
        return ret;
    }
    ret = i2c_driver_install(I2C_NUM_1, conf.mode, 0, 0, 0);
    if (ret != ESP_OK)
    {
        printf("Error when install i2c\n");
    }
    state = READY;
    return ret;
}

void print_port_data(io_data_t buff)
{
    for (uint8_t port = 0; port < IN_PORT_NUM; port++)
    {
        printf("port I_%d :", port);
        for (int pin = 0; pin < 8; pin++)
        {
            printf("%d ", buff.input[port][pin]);
        }
        printf("\n");
    }

    for (uint8_t port = 0; port < OUT_PORT_NUM; port++)
    {
        printf("port O_%d :", port);
        for (int pin = 0; pin < 8; pin++)
        {
            printf("%d ", buff.output[port][pin]);
        }
        printf("\n");
    }
    printf("\033[2J\033[;H");
}

void reset_port()
{
    output_command_t command;
    for (uint8_t port = 0; port < IN_PORT_NUM; port++)
    {
        printf("reset port %d \n", port);
        for (uint8_t pin = 0; pin < 8; pin++)
        {
            {
                command.port = port,
                command.pin = pin,
                command.state = false,
                pcf8574_write_output(command);
                vTaskDelay(5 / portTICK_PERIOD_MS);
            }
        }
        pcf8574_read_port(port);
    }
}

void io_test()
{
    output_command_t command;
    for (uint8_t port = 0; port < IN_PORT_NUM; port++)
    {
        for (int pin = 0; pin < 8; pin++)
        {
            {
                command.port = port,
                command.pin = pin,
                command.state = true,
                pcf8574_write_output(command);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
        }
    }
    for (uint8_t port = 0; port < IN_PORT_NUM; port++)
    {
        for (int pin = 0; pin < 8; pin++)
        {
            {
                command.port = port,
                command.pin = pin,
                command.state = false,
                pcf8574_write_output(command);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
        }
    }
}

uint8_t io_read_pin_value(uint8_t port, uint8_t pin)
{
    return io_data.input[port][pin];
}

void command_enqueue(output_command_t *command)
{
    if (xQueueSend(command_queue, command, 0) != pdTRUE)
    {
        printf("Failed to send command to queue.\n");
    }
}

void queue_init()
{
    command_queue = xQueueCreate(10, sizeof(output_command_t));
    if (command_queue == NULL)
    {
        printf("Failed to create queue.\n");
    }
}

void io_scan_task(void *pvParameter)
{
    init_i2c();
    reset_port();
    queue_init();
    while (1)
    {
        output_command_t command;
        if (xQueueReceive(command_queue, &command, 0) == pdTRUE)
        {
            pcf8574_write_output(command);
        }
        for (uint8_t i = 0; i < IN_PORT_NUM; i++)
        {
            pcf8574_read_port(i);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void io_scan_task_start(void)
{
    if (state == BOOT_UP)
    {
        // Initialize the gate driver instance
        io_driver_instance.io_driver_get_state = io_driver_get_state;
        io_driver_instance.get_io_data = get_io_data;
        io_driver_instance.print_port_data = print_port_data;
        io_driver_instance.command_enqueue = command_enqueue;
        io_driver_instance.io_scan_task = io_scan_task;
        io_driver_instance.io_test = io_test;
        io_driver_instance.io_read_pin_value = io_read_pin_value;
    }

    xTaskCreate(io_scan_task, "io_scan_task", 4096, NULL, 5, NULL);
}

io_driver_t *get_io_driver_instance()
{
    return &io_driver_instance;
}
