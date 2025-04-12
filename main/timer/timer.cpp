#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "driver/gptimer_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "hal/timer_types.h"
#include "soc/clk_tree_defs.h"

#define LED_RED GPIO_NUM_23   // Red LED on GPIO 23
#define LED_BLUE GPIO_NUM_22   // Blue LED on GPIO 22
#define LED_GREEN GPIO_NUM_1  // Blue LED on GPIO 1

static const char *TAG = "TIMER";

// Global timer handles
gptimer_handle_t timer1 = NULL;
gptimer_handle_t timer23 = NULL;
gptimer_handle_t timer4 = NULL;

// Forward declarations of ISR functions
static bool timer1_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data);
static bool timer23_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data);
static bool timer4_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data);

/**
 * Timer interrupt service routine for Timer1 (equivalent to 16-bit timer)
 * This blinks the blue LED at 6kHz (3kHz square wave)
 */
static bool timer1_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data) {
    // Toggle blue LED
    static int level = 0;
    level = !level;
    gpio_set_level(LED_BLUE, level);
    return true;  // Return true to keep the timer running
}

/**
 * Timer interrupt service routine for Timer23 (equivalent to 32-bit timer)
 * This blinks the red LED at 0.5Hz (1s ON, 1s OFF)
 */
static bool timer23_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data) {
    // Toggle red LED
    static int level = 0;
    level = !level;
    gpio_set_level(LED_RED, level);
    return true;  // Return true to keep the timer running
}

/**
 * Timer interrupt service routine for Timer4
 * This blinks the green LED
 **/
static bool timer4_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data) {
    // Toggle the green LED
    static int level = 0;
    level = !level;
    gpio_set_level(LED_GREEN, level);
    return true;
}

/**
 * Initializes Timer1 to toggle a blue LED at high frequency (equivalent to 16-bit timer)
 */
void InitTimer1(void) {
    // Configure the LED GPIO as an output
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << LED_BLUE);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    // Create timer1 (high frequency)
    gptimer_config_t timer_config = {};
    timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000000;  // 1MHz (1us resolution)
    timer_config.intr_priority = 0;  // Using default priority
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer1));

    // Configure the alarm callback
    gptimer_event_callbacks_t cbs = {};
    cbs.on_alarm = timer1_isr;      // Assign our ISR function to the alarm event (when the timer reaches the alarm count)
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer1, &cbs, NULL));

    // Configure alarm to trigger at ~6kHz (166us period)
    gptimer_alarm_config_t alarm_config = {};
    alarm_config.reload_count = 0;
    alarm_config.alarm_count = 166;  // Fire ISR every 166us (~6kHz)
    alarm_config.flags.auto_reload_on_alarm = true;
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer1, &alarm_config));

    // Enable and start the timer
    ESP_ERROR_CHECK(gptimer_enable(timer1));
    ESP_ERROR_CHECK(gptimer_start(timer1));

    ESP_LOGI(TAG, "Timer1 initialized (6kHz)");
}

/**
 * Initializes Timer23 to toggle a red LED at 0.5Hz (equivalent to 32-bit timer)
 */
void InitTimer23(void) {
    // Configure the LED GPIO as an output
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << LED_RED);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    // Create timer23 (low frequency)
    gptimer_config_t timer_config = {};
    timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000000;  // 1MHz (1us resolution)
    timer_config.intr_priority = 0;  // Using default priority
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer23));

    // Configure the alarm callback
    gptimer_event_callbacks_t cbs = {};
    cbs.on_alarm = timer23_isr;
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer23, &cbs, NULL));

    // Configure alarm to trigger at 0.5Hz (1s ON, 1s OFF = 2s period)
    gptimer_alarm_config_t alarm_config = {};
    alarm_config.reload_count = 0;
    alarm_config.alarm_count = 1000000;  // Fire ISR every 1s
    alarm_config.flags.auto_reload_on_alarm = true;
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer23, &alarm_config));

    // Enable and start the timer
    ESP_ERROR_CHECK(gptimer_enable(timer23));
    ESP_ERROR_CHECK(gptimer_start(timer23));

    ESP_LOGI(TAG, "Timer23 initialized (0.5Hz)");
}

/**
 * Initializes Timer4 to toggle a green LED
 */
void InitTimer4(void) {
    // Configure the LED GPIO as an output
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << LED_GREEN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Create timer4
    gptimer_config_t timer_config = {};
    timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000000;
    timer_config.intr_priority = 0;
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer4));

    // Configure alarm callback
    gptimer_event_callbacks_t cbs = {};
    cbs.on_alarm = timer4_isr;
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer4, &cbs, NULL));

    // Configure alarm to trigger
    gptimer_alarm_config_t alarm_config = {};
    alarm_config.reload_count = 0;
    alarm_config.alarm_count = 500000;
    alarm_config.flags.auto_reload_on_alarm = true;
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer4, &alarm_config));

    // Enable and start the timer
    ESP_ERROR_CHECK(gptimer_enable(timer4));
    ESP_ERROR_CHECK(gptimer_start(timer4));

    ESP_LOGI(TAG, "TIMER4 initialized (1KHz)");
}

