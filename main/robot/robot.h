#ifndef ROBOT_H
#define ROBOT_H

#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"

// Motor control functions
esp_err_t mcpwm_motor_init(void);
esp_err_t mcpwm_motor_control(uint8_t motor_num, int speed);
void mcpwm_motor_stop(uint8_t motor_num);

#endif
