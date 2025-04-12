#include <stdio.h>
#include "LED_Blink.h"
#include "timer/timer.h"
#include "robot/robot.h"

extern "C" void app_main(void) {
    // Control LED (timer based)
    InitTimer1();
    InitTimer23();
    InitTimer4();
    // Control Motors
    mcpwm_motor_init();
    mcpwm_motor_control(1, -250);
}
