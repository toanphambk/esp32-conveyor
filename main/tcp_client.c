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

void send_data(int data)
{
    char str[10] = "";
    sprintf(str, "%d\n", data);
    send(sock_fd, str, strlen(str), 0);
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

    sock_fd = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock_fd < 0)
    {
        printf("Unable to create socket: errno %d", errno);
    }
    printf("Socket created, connecting to %s:%d", host_ip, PORT);

    int err = connect(sock_fd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
    if (err != 0)
    {
        if (sock_fd != -1)
        {
            shutdown(sock_fd, 0);
            close(sock_fd);
        }
        printf("Socket unable to connect: errno %d", errno);
    }

    printf("Successfully connected");
}

void tcp_client_start_task(void)
{
    tcp_client_tx_task(NULL);
}