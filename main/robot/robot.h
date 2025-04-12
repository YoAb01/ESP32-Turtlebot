#ifndef ROBOT_H
#define ROBOT_H

#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"

// Motor control functions
esp_err_t mcpwm_motor_init(void);
esp_err_t mcpwm_motor_control(uint8_t motor_num, int speed);
void mcpwm_motor_stop(uint8_t motor_num);

// Elemental moves
void robot_move_forward(int speed);
void robot_move_backward(int speed);
void robot_turn_right(int speed);
void robot_turn_left(int speed);
void robot_full_stop();

#endif
