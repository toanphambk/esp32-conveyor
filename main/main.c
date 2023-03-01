#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "io_driver.h"
#include "gate_driver.h"
#include "esp_heap_trace.h"
#include "esp_log.h"

//dungnt98
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi_station.h"
#include "tcp_client.h"
#include "encoder.h"

static const char *TAG = "memory_status";
gate_driver_setting_t gate_setting_array[2] = {
    {.right = {
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
     }},
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

void heap_trace()
{
    esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_level_set("heap_init", ESP_LOG_DEBUG);
    heap_trace_start(HEAP_TRACE_ALL);
    heap_trace_dump();
    size_t free_heap_size = esp_get_minimum_free_heap_size();
    ESP_LOGI(TAG, "Free heap size: %d bytes", free_heap_size);
}

void app_main()
{
    /* have to push those lines here */
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    start_wifi_station();
    tcp_client_start();

    // xTaskCreatePinnedToCore(&io_scan_task, "io_scan", 4096, NULL, 5, NULL, 0);
    gate_driver_t *gate_driver = get_gate_driver_instance();
    gate_driver->gate_driver_init(gate_setting_array, 2);
    gate_driver->print_gate_driver_setting();
    // output_command_t command;
    // static io_data_t io_data;
    
    while (1)
    {
        // for (uint8_t port = 0; port < IN_PORT_NUM; port++)
        // {
        //     for (int pin = 0; pin < 8; pin++)
        //     {
        //         get_io_data(&io_data);
        //         print_port_data(io_data);
        //         vTaskDelay(SCAN_INTERVAL / portTICK_PERIOD_MS);
        //         {
        //             command.port = port;
        //             2 command.pin = pin;
        //             command.state = !io_data.output[port][pin];
        //             command_enqueue(&command);
        //         }
        //     }
        // }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
