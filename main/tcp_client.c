#include "tcp_client.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

#define HOST_IP_ADDR "192.168.1.4"
#define PORT 3333

static const char *TAG = "example";
static const char *payload = "Message from ESP32 ";

static QueueHandle_t tcp_command_queue;

void tcp_queue_init()
{
    tcp_command_queue = xQueueCreate(10, sizeof(tcp_msg_t));
    if (tcp_command_queue == NULL)
    {
        printf("Failed to create queue.\n");
    }
}

void tcp_send_data(char* data, uint32_t len)
{
    tcp_msg_t msg;

    if (len >= TCP_MSG_MAX_LEN) {
        printf("msg too long\r\n");
        return;
    }

    memset(&msg, 0, sizeof(msg));
    memcpy(msg.data, data, len);
    msg.len = len;

    if (xQueueSend(tcp_command_queue, &msg, 0) != pdTRUE)
    {
        printf("Failed to send command to tcp queue.\n");
    }
}

static void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;
    tcp_msg_t msg_to_send;

    while (1)
    {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(host_ip);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
        if (err != 0)
        {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Successfully connected");

        while (1)
        {
            /* tcp send data */
            memset(&msg_to_send, 0, sizeof(msg_to_send));
            if (xQueueReceive(tcp_command_queue, &msg_to_send, 0) == pdTRUE)
            {
                int err = send(sock, payload, strlen(payload), 0);
                if (err < 0)
                {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
            }

            /* tcp read data */
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0)
            {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            }
            else
            {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
            }

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

void tcp_client_start(void)
{
    tcp_queue_init();
    
    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}