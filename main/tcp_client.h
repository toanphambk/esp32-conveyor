#pragma once
#include <stdint.h>

#define TCP_MSG_MAX_LEN 256
#define HOST_IP_ADDR "192.168.0.102"
#define PORT 3333
#define TCP_MSG_MAX_LEN 256
typedef struct
{
    char data[TCP_MSG_MAX_LEN];
    uint32_t len;
} tcp_msg_t;

void tcp_client_start_task(void);
void send_tcp_data(int data);