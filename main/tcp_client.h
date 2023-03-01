#pragma once
#include <stdint.h>

#define TCP_MSG_MAX_LEN 256

typedef struct {
    char data[TCP_MSG_MAX_LEN];
    uint32_t len;
} tcp_msg_t;

void tcp_client_start(void);
void tcp_send_data(char* data, uint32_t len);