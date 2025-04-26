#include "connection/wifi_conn.h"
#include "esp_log.h"
#include "robot/robot.h"
#include <stdio.h>
#include <algorithm>
#include "driver/gpio.h"
#include <string.h>
#include "esp_spiffs.h"

#define PORT 4210
#define MAX_BUF 128

static const char *WIFI_TAG = "wifi_conn";

/* WiFi Station Event Handler */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    static int s_retry_num = 0;
    if (s_retry_num < MAX_RETRY) {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI(WIFI_TAG, "retry to connect to the AP");
    } else {
      ESP_LOGI(WIFI_TAG, "Failed to connect after %d attempts", MAX_RETRY);
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI(WIFI_TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
  }
}

void wifi_init_sta(void)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      &instance_got_ip));

  wifi_config_t wifi_config = {};
  strcpy((char *)wifi_config.sta.ssid, WIFI_SSID_CONN);
  strcpy((char *)wifi_config.sta.password, WIFI_PASS_CONN);
  wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(WIFI_TAG, "wifi_init_sta finished");
  ESP_LOGI(WIFI_TAG, "Connecting to %s...", WIFI_SSID_CONN);
}

void udp_server_task_conn(void *pvParameters) {
  char rx_buffer[MAX_BUF];
  struct sockaddr_in6 source_addr;
  socklen_t socklen = sizeof(source_addr);

  struct sockaddr_in6 dest_addr;
  dest_addr.sin6_family = AF_INET6;
  dest_addr.sin6_port = htons(PORT);
  dest_addr.sin6_addr = in6addr_any;

  int sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
  if (sock < 0) {
    ESP_LOGE(WIFI_TAG, "Unable to create socket: errno %d", errno);
    vTaskDelete(NULL);
    return;
  }

  if (bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
    ESP_LOGE(WIFI_TAG, "Socket unable to bind: errno %d", errno);
    close(sock);
    vTaskDelete(NULL);
    return;
  }

  ESP_LOGI(WIFI_TAG, "UDP server listening on port %d", PORT);

  // Define GPIO pins for LEDs
  const int LED_PIN_RED = 23;
  const int LED_PIN_BLUE = 22;
  const int LED_PIN_GREEN = 21;
  const int LED_PIN_YELLOW = 19;

  // Initialize LED pins as outputs
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << LED_PIN_RED) | (1ULL << LED_PIN_BLUE) | 
                        (1ULL << LED_PIN_GREEN) | (1ULL << LED_PIN_YELLOW);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);

  while (true) {
    int len = recvfrom(sock, rx_buffer, MAX_BUF - 1, 0, (struct sockaddr *)&source_addr, &socklen);

    if (len < 0) {
      ESP_LOGE(WIFI_TAG, "recvfrom failed: errno %d", errno);
      break;
    }

    rx_buffer[len] = 0; // Null terminate
    ESP_LOGI(WIFI_TAG, "Received packet: %s", rx_buffer);

    // Handle PING request for connection status
    if (strcmp(rx_buffer, "PING") == 0) {
      // Send back PONG response
      const char* pong_response = "PONG";
      sendto(sock, pong_response, strlen(pong_response), 0, 
             (struct sockaddr*)&source_addr, sizeof(source_addr));
      ESP_LOGI(WIFI_TAG, "Responded to PING with PONG");
      continue;  // Continue to next iteration
    }

    // Check if it's a joystick command (two numbers separated by comma)
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

      ESP_LOGI(WIFI_TAG, "Motor control: L=%d, R=%d", left_speed, right_speed);
    }
    // Check if it's an LED command (format: "LED color ON/OFF")
    else if (strncmp(rx_buffer, "LED ", 4) == 0) {
      char color[10];
      char state[5];

      if (sscanf(rx_buffer, "LED %s %s", color, state) == 2) {
        int pin = -1;
        bool turn_on = (strcmp(state, "ON") == 0);

        if (strcmp(color, "red") == 0) {
          pin = LED_PIN_RED;
        } else if (strcmp(color, "blue") == 0) {
          pin = LED_PIN_BLUE;
        } else if (strcmp(color, "green") == 0) {
          pin = LED_PIN_GREEN;
        } else if (strcmp(color, "yellow") == 0) {
          pin = LED_PIN_YELLOW;
        }

        if (pin != -1) {
          gpio_set_level((gpio_num_t)pin, turn_on ? 1 : 0);
          ESP_LOGI(WIFI_TAG, "Set LED %s to %s", color, turn_on ? "ON" : "OFF");
        } else {
          ESP_LOGW(WIFI_TAG, "Unknown LED color: %s", color);
        }
      }
    }
    // Check if it's an RGB LED command (format: "RGB r g b")
    else if (strncmp(rx_buffer, "RGB ", 4) == 0) {
      int r, g, b;
      if (sscanf(rx_buffer, "RGB %d %d %d", &r, &g, &b) == 3) {
        gpio_set_level((gpio_num_t)LED_PIN_RED, r > 127 ? 1 : 0);
        gpio_set_level((gpio_num_t)LED_PIN_GREEN, g > 127 ? 1 : 0);
        gpio_set_level((gpio_num_t)LED_PIN_BLUE, b > 127 ? 1 : 0);
        ESP_LOGI(WIFI_TAG, "RGB command: R=%d, G=%d, B=%d", r, g, b);
      }
    } else {
        ESP_LOGW(WIFI_TAG, "Unknown command format: %s", rx_buffer);
    }
  }
  close(sock);
  vTaskDelete(NULL);
}
