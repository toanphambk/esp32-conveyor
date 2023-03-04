#ifndef _APP_PROCESS_DARTA_H_
#define _APP_PROCESS_DARTA_H_

#include <stdint.h>
#include "cJSON.h"

typedef void (*handler_t)(cJSON *jsRoot);

typedef struct
{
    handler_t _handler;
    uint32_t _cmd_number;
} pd_handler_t;

void app_process_data_loop(void);

#endif /* _APP_PROCESS_DARTA_H_ */
