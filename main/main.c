#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "io_driver.h"
#include "gate_driver.h"
#include "esp_heap_trace.h"
#include "esp_log.h"

// dungnt98
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi_station.h"
#include "tcp_client.h"
#include "encoder.h"
#include "process_data.h"

static const char *TAG = "memory_status";

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

    /* io driver & gate driver */
    io_scan_task_start();
    gate_driver_start_task();

    /* wifi and tcp */
    start_wifi_station();
    tcp_client_start_task();

    while (1)
    {
        app_process_data_loop();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
