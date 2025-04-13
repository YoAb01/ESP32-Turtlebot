#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "driver/gptimer.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/timer_types.h"

static bool trigger_timer_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data);
void init_ultrasonic();
static void echo_isr_handler(void* arg);
void trigger_ultrasonic();
float get_distance_cm();

#endif
