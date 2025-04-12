#ifndef WIFI_AP_H
#define WIFI_AP_H

#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types_generic.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include <cstring>
#include "string.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#define WIFI_SSID "ESP32_2WD_Robot"
#define WIFI_PASS "TestRobotESP32"
#define WIFI_CHANNEL 1
#define MAX_STA_CONN 1

void wifi_init_ap(void);

#endif
