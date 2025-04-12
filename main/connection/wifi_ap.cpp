#include "connection/wifi_ap.h"
#include "robot/robot.h"


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
    strcpy((char *)ap_config.ap.ssid, "ESP32_ROBOT");
    strcpy((char *)ap_config.ap.password, "TestRobotESP32");
    ap_config.ap.ssid_len = strlen((char *)ap_config.ap.ssid);
    ap_config.ap.max_connection = 1;
    ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    ap_config.ap.channel = 1;

    if (strlen((char *)ap_config.ap.password) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("WIFI_AP", "Access Point started! SSID: %s, Password: %s", ap_config.ap.ssid, ap_config.ap.password);
}
