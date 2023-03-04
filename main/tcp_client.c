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

#include "cJSON.h"

QueueHandle_t tcp_tx_queue;
QueueHandle_t tcp_rx_queue;
int sock_fd;

void tcp_queue_init()
{
    tcp_tx_queue = xQueueCreate(10, sizeof(tcp_msg_t));
    if (tcp_tx_queue == NULL)
    {
        printf("Failed to create queue.\n");
    }

    tcp_rx_queue = xQueueCreate(10, sizeof(tcp_msg_t));
    if (tcp_rx_queue == NULL)
    {
        printf("Failed to create queue.\n");
    }
}

void tcp_send_data(char *data, uint32_t len)
{
    if (len >= TCP_MSG_MAX_LEN)
    {
        printf("msg too long\r\n");
        return;
    }

    tcp_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    memcpy(msg.data, data, len);
    msg.len = len;

    if (xQueueSend(tcp_tx_queue, &msg, 0) != pdTRUE)
    {
        printf("Failed to send command to tcp queue.\n");
    }
}

static void tcp_client_rx_task(void *pvParameters)
{
    tcp_msg_t rx_msg;

    while (1)
    {
        /* tcp read data */
        memset(&rx_msg, 0, sizeof(rx_msg));
        rx_msg.len = recv(sock_fd, rx_msg.data, sizeof(rx_msg.data) - 1, 0);
        if (rx_msg.len > 0)
        {
            rx_msg.data[rx_msg.len] = 0; // Null-terminate whatever we received and treat like a string
            if (xQueueSend(tcp_rx_queue, &rx_msg, 0) != pdPASS)
            {
                printf("Failed to send command to tcp tcp_rx_queue.\n");
            }

            // tcp_send_data(rx_msg.data, rx_msg.len);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

static void tcp_client_tx_task(void *pvParameters)
{
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;
    struct sockaddr_in dest_addr;
    tcp_msg_t tx_msg;

    dest_addr.sin_addr.s_addr = inet_addr(host_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    while(1) {
        sock_fd = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock_fd < 0)
        {
            printf("Unable to create socket: errno %d", errno);
        }
        printf("Socket created, connecting to %s:%d", host_ip, PORT);

        int err = connect(sock_fd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
        if (err != 0)
        {
            if (sock_fd != -1) {
                shutdown(sock_fd, 0);
                close(sock_fd);
            }
            printf("Socket unable to connect: errno %d", errno);
        } 
        else 
        {
            printf("Successfully connected");
            send(sock_fd, "dungnt98", strlen("dungnt98"), 0);
            xTaskCreate(tcp_client_rx_task, "tcp_client_tx_task", 4096, NULL, 5, NULL);
            break;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    while (1)
    {
        /* tcp send data */
        memset(&tx_msg, 0, sizeof(tx_msg));
        if (xQueueReceive(tcp_tx_queue, &tx_msg, 0) == pdPASS)
        {
            int err = send(sock_fd, tx_msg.data, tx_msg.len, 0);
            if (err < 0)
            {
                printf("Error occurred during sending: errno %d", errno);
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void tcp_client_start_task(void)
{
    tcp_queue_init();

    xTaskCreate(tcp_client_tx_task, "tcp_client_tx_task", 4096, NULL, 5, NULL);
}
