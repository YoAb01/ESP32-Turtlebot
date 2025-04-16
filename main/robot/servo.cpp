
#include "servo.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include <inttypes.h>

static const char* TAG = "LEDC_SERVO";

#define SERVO_MIN_PULSEWIDTH_US 500
#define SERVO_MAX_PULSEWIDTH_US 2500
#define SERVO_MAX_DEGREE        180

#define SERVO_LEDC_TIMER        LEDC_TIMER_0
#define SERVO_LEDC_MODE         LEDC_HIGH_SPEED_MODE
#define SERVO_LEDC_CHANNEL      LEDC_CHANNEL_0
#define SERVO_LEDC_FREQ_HZ      50
#define SERVO_LEDC_RESOLUTION   LEDC_TIMER_13_BIT

static int servo_gpio = -1;
static uint32_t max_duty = 0;

esp_err_t servo_init(int gpio_num) {
  servo_gpio = gpio_num;

  ledc_timer_config_t ledc_timer = {};
  ledc_timer.speed_mode = SERVO_LEDC_MODE;
  ledc_timer.duty_resolution = SERVO_LEDC_RESOLUTION;
  ledc_timer.timer_num = SERVO_LEDC_TIMER;
  ledc_timer.freq_hz = SERVO_LEDC_FREQ_HZ;
  ledc_timer.clk_cfg = LEDC_AUTO_CLK;
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  ledc_channel_config_t ledc_channel = {};
  ledc_channel.speed_mode = SERVO_LEDC_MODE;
  ledc_channel.channel = SERVO_LEDC_CHANNEL;
  ledc_channel.timer_sel = SERVO_LEDC_TIMER;
  ledc_channel.intr_type = LEDC_INTR_DISABLE;
  ledc_channel.gpio_num = gpio_num;
  ledc_channel.duty = 0;
  ledc_channel.hpoint = 0;
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

  max_duty = (1 << SERVO_LEDC_RESOLUTION); // 8192 for 13-bit

  ESP_LOGI(TAG, "Servo initialized on GPIO %d", gpio_num);
  return ESP_OK;
}

esp_err_t servo_set_angle(int angle_deg) {
  if (servo_gpio < 0) {
    ESP_LOGE(TAG, "Servo not initialized!");
    return ESP_FAIL;
  }

  angle_deg = angle_deg < 0 ? 0 : (angle_deg > SERVO_MAX_DEGREE ? SERVO_MAX_DEGREE : angle_deg);

  int pulsewidth_us = SERVO_MIN_PULSEWIDTH_US +
      (angle_deg * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US)) / SERVO_MAX_DEGREE;

  uint32_t duty = (pulsewidth_us * max_duty * SERVO_LEDC_FREQ_HZ) / 1000000;

  ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, 0);
  ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL);

  ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, duty);
  ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL);

  ESP_LOGI(TAG, "Servo angle set to %d°, pulse = %dus, duty = %" PRIu32, angle_deg, pulsewidth_us, duty);

  return ESP_OK;
}
