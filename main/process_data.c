#include "process_data.h"
#include "tcp_client.h"
#include "cJSON.h"
#include "gate_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

extern QueueHandle_t tcp_rx_queue;

/* {"command": %d, "gateNo": %d, "gatePo" : %d} */
void app_control_gate(cJSON *jsRoot)
{
    gate_command_t gate_cmd;

    memset(&gate_cmd, 0, sizeof(gate_cmd));

    cJSON *js_cmd = cJSON_GetObjectItem(jsRoot, "command");
    if (js_cmd == NULL)
    {
        printf("parse json cmd fail\n");
        cJSON_Delete(jsRoot);
        return;
    }
    gate_cmd.command_name = js_cmd->valueint;

    cJSON *js_gate_no = cJSON_GetObjectItem(jsRoot, "gateNo");
    if (js_gate_no == NULL)
    {
        printf("parse json cmd fail\n");
        cJSON_Delete(jsRoot);
        return;
    }
    gate_cmd.gate_no = js_gate_no->valueint;

    cJSON *js_gate_po = cJSON_GetObjectItem(jsRoot, "gatePo");
    if (js_gate_po == NULL)
    {
        printf("parse json cmd fail\n");
        cJSON_Delete(jsRoot);
        return;
    }
    gate_cmd.position = js_gate_po->valueint;

    get_gate_driver_instance()->send_gate_command(gate_cmd);

}

pd_handler_t array_handler[] = {
    {app_control_gate, GATE_STOP_COMMAND},
    {app_control_gate, GATE_TRIGGER_COMMAND},
    {app_control_gate, GATE_CLOSE_COMMAND},
    {app_control_gate, GATE_OPEN_COMMAND},
};

/* {"command": %d, "gateNo": %d, "gatePo" : %d, "board": 1} */
void app_process_data_loop(void)
{
    tcp_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    if (xQueueReceive(tcp_rx_queue, &msg, 0) == pdTRUE)
    {
        printf("tcp receive:%s\r\n", msg.data);

        cJSON *jsRoot = cJSON_Parse(msg.data);
        if (!jsRoot)
            return;

        cJSON *js_cmd = cJSON_GetObjectItem(jsRoot, "command");
        if (js_cmd == NULL)
        {
            printf("parse json cmd fail\n");
            cJSON_Delete(jsRoot);
            return;
        }

        for (uint32_t i = 0; i < sizeof(array_handler)/sizeof(array_handler[0]); i++)
        {
            if (array_handler[i]._cmd_number == js_cmd->valueint)
            {
                array_handler[i]._handler(jsRoot);
            }
        }

        cJSON_Delete(jsRoot);
    }
}
