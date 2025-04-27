#include "connection/wifi_conn.h"
#include "esp_log.h"
#include "robot/robot.h"
#include <stdio.h>
#include <algorithm>
#include "driver/gpio.h"
#include <string.h>
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PORT 4210
#define MAX_BUF 128

// Shift Register Pins
#define DATA_PIN   static_cast<gpio_num_t>(23)  // DS - Serial Data Input
#define CLOCK_PIN  static_cast<gpio_num_t>(22)  // SHCP - Shift Register Clock
#define LATCH_PIN  static_cast<gpio_num_t>(21)  // STCP - Storage Register Clock

static const char *WIFI_TAG = "wifi_conn";

// Define bit positions for each LED in the shift register
#define LED_RED_BIT     0
#define LED_BLUE_BIT    1
#define LED_GREEN_BIT   2
#define LED_YELLOW_BIT  3
#define LED_RGB_R_BIT   4
#define LED_RGB_G_BIT   5
#define LED_RGB_B_BIT   6

uint8_t shift_register_state = 0;  // Current output state of the shift register

// Function to write to the shift register
void shift_register_write(uint8_t data) {
  // Ensure latch is low before shifting data
  gpio_set_level(LATCH_PIN, 0);
  // Small delay to ensure latch timing
  vTaskDelay(1 / portTICK_PERIOD_MS);

  // Shift out data bit by bit, MSB first
  for (int i = 7; i >= 0; i--) {
    gpio_set_level(CLOCK_PIN, 0);
    gpio_set_level(DATA_PIN, (data >> i) & 0x01);
    // Small delay for data setup time
    vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(CLOCK_PIN, 1);
    // Small delay for clock pulse width
    vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(CLOCK_PIN, 0);
  }

  // Latch the data to the output
  gpio_set_level(LATCH_PIN, 1);
  vTaskDelay(1 / portTICK_PERIOD_MS);
  gpio_set_level(LATCH_PIN, 0);

  ESP_LOGI(WIFI_TAG, "Shift register updated with value: 0x%02X", data);
}

void shift_register_init() {
  // Configure pins
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << DATA_PIN) | (1ULL << CLOCK_PIN) | (1ULL << LATCH_PIN);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);

  // Set initial pin states
  gpio_set_level(DATA_PIN, 0);
  gpio_set_level(CLOCK_PIN, 0);
  gpio_set_level(LATCH_PIN, 0);

  // Reset shift register state and send to device
  shift_register_state = 0;
  shift_register_write(shift_register_state);

  ESP_LOGI(WIFI_TAG, "Shift register initialized");
}

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

  // Initialize shift register
  shift_register_init();

  while (true) {
    int len = recvfrom(sock, rx_buffer, MAX_BUF - 1, 0, (struct sockaddr *)&source_addr, &socklen);

    if (len < 0) {
      ESP_LOGE(WIFI_TAG, "recvfrom failed: errno %d", errno);
      break;
    }

    rx_buffer[len] = 0; // Null terminate
    ESP_LOGI(WIFI_TAG, "Received packet: %s", rx_buffer);

    // Handle PING
    if (strcmp(rx_buffer, "PING") == 0) {
      const char* pong_response = "PONG";
      sendto(sock, pong_response, strlen(pong_response), 0, 
              (struct sockaddr*)&source_addr, sizeof(source_addr));
      ESP_LOGI(WIFI_TAG, "Responded to PING with PONG");
      continue;
    }

    // Handle joystick command
    int8_t axis1 = 0, axis2 = 0;
    if (sscanf(rx_buffer, "%hhd,%hhd", &axis1, &axis2) == 2) {
      if (abs(axis1) < 10) axis1 = 0;
      if (abs(axis2) < 10) axis2 = 0;

      int left_speed = (axis1 + axis2) * 2;
      int right_speed = (axis1 - axis2) * 2;

      left_speed = std::max(-255, std::min(255, left_speed));
      right_speed = std::max(-255, std::min(255, right_speed));

      mcpwm_motor_control(1, left_speed);
      mcpwm_motor_control(2, right_speed);

      ESP_LOGI(WIFI_TAG, "Motor control: L=%d, R=%d", left_speed, right_speed);
    }
    // Handle LED commands
    else if (strncmp(rx_buffer, "LED ", 4) == 0) {
      char color[10];
      char state[5];

      if (sscanf(rx_buffer, "LED %s %s", color, state) == 2) {
        bool turn_on = (strcmp(state, "ON") == 0);
        uint8_t old_state = shift_register_state;

        // Debug info before change
        ESP_LOGI(WIFI_TAG, "Before change: Shift register state: 0x%02X", shift_register_state);

        if (strcmp(color, "red") == 0) {
          if (turn_on)
            shift_register_state |= (1 << LED_RED_BIT);
          else
            shift_register_state &= ~(1 << LED_RED_BIT);
        } else if (strcmp(color, "blue") == 0) {
          if (turn_on)
            shift_register_state |= (1 << LED_BLUE_BIT);
          else
            shift_register_state &= ~(1 << LED_BLUE_BIT);
        } else if (strcmp(color, "green") == 0) {
          if (turn_on)
            shift_register_state |= (1 << LED_GREEN_BIT);
          else
            shift_register_state &= ~(1 << LED_GREEN_BIT);
        } else if (strcmp(color, "yellow") == 0) {
          if (turn_on)
            shift_register_state |= (1 << LED_YELLOW_BIT);
          else
            shift_register_state &= ~(1 << LED_YELLOW_BIT);
        } else {
          ESP_LOGW(WIFI_TAG, "Unknown LED color: %s", color);
          continue;
        }

        // Debug info after change
        ESP_LOGI(WIFI_TAG, "After change: Shift register state: 0x%02X", shift_register_state);

        // Only write to the shift register if the state has changed
        if (old_state != shift_register_state) {
          shift_register_write(shift_register_state);
          ESP_LOGI(WIFI_TAG, "Set LED %s to %s", color, turn_on ? "ON" : "OFF");
        } else {
          ESP_LOGI(WIFI_TAG, "LED %s was already %s, no change needed", color, turn_on ? "ON" : "OFF");
        }
      }
    }
    // Handle RGB LED command
    else if (strncmp(rx_buffer, "RGB ", 4) == 0) {
      int r, g, b;
      if (sscanf(rx_buffer, "RGB %d %d %d", &r, &g, &b) == 3) {
        uint8_t old_state = shift_register_state;

        // Update red LED
        if (r > 127)
            shift_register_state |= (1 << LED_RGB_R_BIT);
        else
            shift_register_state &= ~(1 << LED_RGB_R_BIT);

        // Update green LED
        if (g > 127)
            shift_register_state |= (1 << LED_RGB_G_BIT);
        else
            shift_register_state &= ~(1 << LED_RGB_G_BIT);

        // Update blue LED
        if (b > 127)
            shift_register_state |= (1 << LED_RGB_B_BIT);
        else
            shift_register_state &= ~(1 << LED_RGB_B_BIT);

        // Only write if state changed
        if (old_state != shift_register_state) {
            shift_register_write(shift_register_state);
        }

        ESP_LOGI(WIFI_TAG, "RGB command: R=%d, G=%d, B=%d", r, g, b);
      }
    }
    else {
        ESP_LOGW(WIFI_TAG, "Unknown command format: %s", rx_buffer);
    }
  }
  close(sock);
  vTaskDelete(NULL);
}
