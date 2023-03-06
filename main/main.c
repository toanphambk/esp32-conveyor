#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "tcp_client.h"
#include "encoder.h"

void app_main()
{
    /* have to push those lines here */
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_sta();
    tcp_client_start_task();
    encoder_start();
    while (1)
    {
        int data = get_pulse();
        send_tcp_data(data);
    }
}
