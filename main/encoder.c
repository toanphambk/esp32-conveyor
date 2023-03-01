#include "encoder.h"
#include "io_driver.h"
#include <string.h>

static encoder_t g_encoder;

void encoder_init(void)
{
    memset(&g_encoder, 0, sizeof(g_encoder));
}

void encoder_read_pulse_count(void)
{
    io_driver_t *io_instance = get_io_driver_instance();
    io_data_t io_current_value;
    static io_data_t io_old_value;

    memset(&io_current_value, 0, sizeof(io_current_value));
    io_instance->get_io_data(&io_current_value);

    /* clockwise direction: A change frome 1 to 0 but B is still 1. */
    if (io_current_value.input[ENCODER_A_PORT][ENCODER_A_PIN] != io_old_value.input[ENCODER_B_PORT][ENCODER_B_PIN])
    {
        if (io_current_value.input[ENCODER_A_PORT][ENCODER_A_PIN] == 0 && 
            io_current_value.input[ENCODER_B_PORT][ENCODER_B_PIN] == 1)
        {
            io_old_value.input[ENCODER_A_PORT][ENCODER_A_PIN] = io_current_value.input[ENCODER_A_PORT][ENCODER_A_PIN];
            g_encoder.pulse_count++;
        }
    }
}

void encoder_send_pulse_count(void)
{
    // tcp send g_encoder.pulse_count
}