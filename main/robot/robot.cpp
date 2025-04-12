#include "robot.h"
#include "driver/mcpwm.h"
#include "driver/gpio.h"
#include "driver/mcpwm_types_legacy.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/gpio_types.h"

#define MOTOR_1_ENA    GPIO_NUM_26    // Motor 1 enable pin
#define MOTOR_1_IN1    GPIO_NUM_27    // Motor 1 direction pin 1
#define MOTOR_1_IN2    GPIO_NUM_14    // Motor 1 direction pin 2

#define MOTOR_PWM_FREQ 30000          // Motor frequency in hz
#define MOTOR_PWM_RES  255            // 8-bit resolution. Value in [0, 255]

static char* TAG = "MCPWM_MOTOR";


esp_err_t mcpwm_motor_init(void) {
  // Configure the PWM
  mcpwm_config_t pwm_config = {};
  pwm_config.frequency = MOTOR_PWM_FREQ;
  pwm_config.cmpr_a = 0;
  pwm_config.cmpr_b = 0;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  pwm_config.counter_mode = MCPWM_UP_COUNTER;

  ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOTOR_1_ENA));

  // Configure directiuon pins as regular GPIO output
  gpio_config_t io_conf = {};
  io_conf.pin_bit_mask = (1ULL << MOTOR_1_IN1) | (1ULL << MOTOR_1_IN2);
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&io_conf));

  // Initialize PWM for Motor
  ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config));

  ESP_LOGI(TAG, "MCPWM motor control initialized");
  return ESP_OK;
}

esp_err_t mcpwm_motor_control(uint8_t motor_num, int speed) {
  // Handle edge cases
  if (speed > 255) speed = 255;
  if (speed < -255) speed = -255;

  // Convert speed to duty cycle
  float duty_cycle = (abs(speed) / 255.0f) * 100.0f;

  // Set directions pin for motor 1
  if (motor_num == 1) {
    gpio_set_level(MOTOR_1_IN1, speed >= 0 ? 1 : 0);
    gpio_set_level(MOTOR_1_IN2, speed >= 0 ? 0 : 1);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);
  } else if (motor_num == 2) {
    // TODO: Implement this for the second motor
  } else {
    ESP_LOGE(TAG, "nvalid motor number: %d", motor_num);
  }

  ESP_LOGI(TAG, "Motor %d set to speed %d (duty cycle %.1f%%)",
             motor_num, speed, duty_cycle);
    return ESP_OK;
}

void mcpwm_motor_stop(uint8_t motor_num) {
  if (motor_num == 1) {
    gpio_set_level(MOTOR_1_IN1, 0);
    gpio_set_level(MOTOR_1_IN2, 0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
  } else if (motor_num == 2) {
    //TODO: Implement this for the second motor
  }
}
