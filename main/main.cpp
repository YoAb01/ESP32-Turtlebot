#include <stdio.h>
#include "LED_Blink.h"
#include "timer/timer.h"

extern "C" void app_main(void) {
    InitTimer1();
    InitTimer23();
    InitTimer4();
}