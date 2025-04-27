#include "connection/wifi_ap.h"
#include "robot/robot.h"
#include <algorithm>

#define TAG "WIFI_AP"
#define PORT 4210
#define TAG "ESP32_ROBOT"
#define MAX_BUF 128


void wifi_init_ap() {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t ap_config = {};
    strcpy((char *)ap_config.ap.ssid, WIFI_SSID);
    strcpy((char *)ap_config.ap.password, WIFI_PASS);
    ap_config.ap.ssid_len = strlen((char *)ap_config.ap.ssid);
    ap_config.ap.max_connection = MAX_STA_CONN;
    ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    ap_config.ap.channel = WIFI_CHANNEL;

    if (strlen((char *)ap_config.ap.password) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("WIFI_AP", "Access Point started! SSID: %s, Password: %s", ap_config.ap.ssid, ap_config.ap.password);
}

void udp_server_task_ap(void *pvParameters) {
    char rx_buffer[MAX_BUF];
    struct sockaddr_in6 source_addr;
    socklen_t socklen = sizeof(source_addr);

    struct sockaddr_in6 dest_addr;
    dest_addr.sin6_family = AF_INET6;
    dest_addr.sin6_port = htons(PORT);
    dest_addr.sin6_addr = in6addr_any;

    int sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    if (bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "UDP server listening on port %d", PORT);

    while (true) {
        int len = recvfrom(sock, rx_buffer, MAX_BUF - 1, 0, (struct sockaddr *)&source_addr, &socklen);

        if (len < 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            break;
        }

        rx_buffer[len] = 0;
        int8_t axis1 = 0, axis2 = 0;
        if (sscanf(rx_buffer, "%hhd,%hhd", &axis1, &axis2) == 2) {
              if (abs(axis1) < 10) axis1 = 0;
              if (abs(axis2) < 10) axis2 = 0;

              // Combine forward/backward with turning
              int left_speed = (axis1 + axis2) * 2;
              int right_speed = (axis1 - axis2) * 2;

              // Clamp to valid PWM range [-255, 255]
              left_speed = std::max(-255, std::min(255, left_speed));
              right_speed = std::max(-255, std::min(255, right_speed));

              mcpwm_motor_control(1, left_speed);
              mcpwm_motor_control(2, right_speed);
        } else {
            ESP_LOGW(TAG, "Invalid packet: %s", rx_buffer);
        }
    }

    close(sock);
    vTaskDelete(NULL);
}
