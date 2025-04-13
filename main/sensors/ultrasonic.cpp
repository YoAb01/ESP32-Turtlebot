#include "ultrasonic.h"

#define TRIG_GPIO GPIO_NUM_16
#define ECHO_GPIO GPIO_NUM_17

static const char* TAG = "ULTRASONIC";

static int64_t echo_start_time = 0;
static volatile float last_distance_cm = -1.0;
static gptimer_handle_t trigger_timer = NULL;
static gptimer_handle_t echo_timer = NULL;


static bool trigger_timer_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data) {
  static bool trigger_state = false;
  trigger_state = !trigger_state;
  gpio_set_level(TRIG_GPIO, trigger_state);
  return true;
}

static void IRAM_ATTR echo_isr_handler(void *arg) {
  int level = gpio_get_level(ECHO_GPIO);
  int64_t now = esp_timer_get_time();

  if (level == 1) {
    echo_start_time = now;
  } else {
    int64_t pulse_direction = now - echo_start_time;
    last_distance_cm = (pulse_direction * 343 * 0.0001) / 2.0; // Speed of sound in air 343 m/s converted to cm/us
  }
}

void init_ultrasonic() {
  // Configure trigger pin
  gpio_config_t trig_conf = {};
  trig_conf.intr_type = GPIO_INTR_DISABLE;
  trig_conf.mode = GPIO_MODE_OUTPUT;
  trig_conf.pin_bit_mask = (1ULL << TRIG_GPIO);
  trig_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  trig_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&trig_conf);
  gpio_set_level(TRIG_GPIO, 0);

  // Configure echo pin
  gpio_config_t echo_conf = {};
  echo_conf.intr_type = GPIO_INTR_ANYEDGE;  // Interrupt on both rising and falling edge
  echo_conf.mode = GPIO_MODE_INPUT;
  echo_conf.pin_bit_mask = (1ULL << ECHO_GPIO);
  echo_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  echo_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_config(&echo_conf);

  // Install GPIO ISR service and add handler for echo pin
  gpio_install_isr_service(0);
  gpio_isr_handler_add(ECHO_GPIO, echo_isr_handler, NULL);

  // Initialize and configure timers
  gptimer_config_t trigger_timer_config = {};
  trigger_timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
  trigger_timer_config.direction = GPTIMER_COUNT_UP;
  trigger_timer_config.resolution_hz = 1000000;
  ESP_ERROR_CHECK(gptimer_new_timer(&trigger_timer_config, &trigger_timer));
  gptimer_event_callbacks_t cbs = {};
  cbs.on_alarm = trigger_timer_isr;
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(trigger_timer, &cbs, NULL));

  // Configure alarm
  gptimer_alarm_config_t trigger_alarm_config = {};
  trigger_alarm_config.reload_count = 0;
  trigger_alarm_config.alarm_count = 5000;
  trigger_alarm_config.flags.auto_reload_on_alarm = true;
  ESP_ERROR_CHECK(gptimer_set_alarm_action(trigger_timer, &trigger_alarm_config));
  ESP_LOGI(TAG, "Ultrasonic sensor initialized with GPTimers");
}

void trigger_ultrasonic(void) {
  ESP_ERROR_CHECK(gptimer_enable(trigger_timer));
  ESP_ERROR_CHECK(gptimer_start(trigger_timer));
  ESP_LOGI(TAG, "Triggering ultrasonic sensor");
}

float get_distance_cm() {
  return last_distance_cm;
}




