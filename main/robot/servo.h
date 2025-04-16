#ifndef SERVO_H
#define SERVO_H

#include "esp_err.h"


esp_err_t servo_init(int gpio_num);
esp_err_t servo_set_angle(int angle_deg);

#endif
