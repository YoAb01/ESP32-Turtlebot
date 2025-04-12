#include <stdio.h>
#include "LED_Blink.h"
#include "timer/timer.h"
#include "robot/robot.h"
#include "freertos/FreeRTOS.h"

#define MOTOR_SPEED 200
static char* TAG = "MCPWM_MOTOR";

void test_robot_movement(void *pvParameter) {
    while (true) {
      // Move forward
      ESP_LOGI(TAG, "Moving forward...");
      mcpwm_motor_control(1, MOTOR_SPEED);
      mcpwm_motor_control(2, MOTOR_SPEED);
      vTaskDelay(2000 / portTICK_PERIOD_MS);

      // Move backward
      ESP_LOGI(TAG, "Moving backward...");
      mcpwm_motor_control(1, -MOTOR_SPEED);
      mcpwm_motor_control(2, -MOTOR_SPEED);
      vTaskDelay(2000 / portTICK_PERIOD_MS);

      // Turn left (motor 1 forward, motor 2 backward)
      ESP_LOGI(TAG, "Turning left...");
      mcpwm_motor_control(1, MOTOR_SPEED);
      mcpwm_motor_control(2, -MOTOR_SPEED);
      vTaskDelay(1500 / portTICK_PERIOD_MS);

      // Turn right (motor 1 backward, motor 2 forward)
      ESP_LOGI(TAG, "Turning right...");
      mcpwm_motor_control(1, -MOTOR_SPEED);
      mcpwm_motor_control(2, MOTOR_SPEED);
      vTaskDelay(1500 / portTICK_PERIOD_MS);

      // Full 360-degree turn (left turn for a specific time)
      ESP_LOGI(TAG, "Full 360-degree turn...");
      mcpwm_motor_control(1, MOTOR_SPEED);
      mcpwm_motor_control(2, -MOTOR_SPEED);
      vTaskDelay(3000 / portTICK_PERIOD_MS);

      // Stop the motors after testing
      mcpwm_motor_stop(1);
      mcpwm_motor_stop(2);

      vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main(void) {
    // Control LED (timer based)
    InitTimer1();
    InitTimer23();
    InitTimer4();
    // Control Motors
    mcpwm_motor_init();
    xTaskCreate(test_robot_movement, "robot_movement_task", 2048, NULL, 5, NULL);
}

