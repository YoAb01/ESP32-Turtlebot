#include <stdio.h>
#include "LED_Blink.h"
#include "timer/timer.h"
#include "robot/robot.h"
#include "connection/wifi_ap.h"
#include "sensors/ultrasonic.h"
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

static void ultrasonic_monitor_task(void *pvParameters) {
  vTaskDelay(pdMS_TO_TICKS(500));
  while (1) {
    float distance = get_distance_cm();

    printf("Distance: %.2f cm\n", distance);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

extern "C" void app_main(void) {
    // Control LED (timer based)
    InitTimer1();
    InitTimer23();
    InitTimer4();
    // Control Robot via joystick
    mcpwm_motor_init();
    wifi_init_ap();
    xTaskCreate(udp_server_task, "udp_server_task", 4096, NULL, 5, NULL);

    // Ultrasonci sensor task
    init_ultrasonic();
    trigger_ultrasonic();
    xTaskCreate(ultrasonic_monitor_task, "ultrasonic_monitor", 2048, NULL, 3, NULL);
}

