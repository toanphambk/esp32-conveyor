#ifndef WIFI_H_
#define WIFI_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define EXAMPLE_WIFI_SSID "Toanphambk"
#define EXAMPLE_WIFI_PASS "123123123"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
void wifi_init_sta(void);

#endif /* WIFI_H_ */
