#ifndef WIFI_CONN_H
#define WIFI_CONN_H

#include "wifi_config.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_wifi_default.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "wifi_ap.h"
#include <cstring>

//NOTE: Fill in your Wifi credential here (and comment wifi_config.h above #L4)
/*
#define WIFI_SSID_CONN ""
#define WIFI_PASS_CONN ""
*/

#define MAX_RETRY 5

void heartbeat_task(void *pvParameters);
void wifi_init_sta(void);
void udp_server_task_conn(void *pvParameters);

#endif    // WIFI_CONN_H
