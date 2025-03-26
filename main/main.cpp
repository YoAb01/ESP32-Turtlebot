#include <stdio.h>
#include "LED_Blink.h"

extern "C" void app_main(void) {
    LEDBlink led(GPIO_NUM_2, 500); // Use GPIO2 and 500ms delay

    while (1) {
        led.blink();
    }
}