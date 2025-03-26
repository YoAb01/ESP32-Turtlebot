#ifndef LEDBLINK_H
#define LEDBLINK_H

#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class LEDBlink {
private:
    gpio_num_t pin;
    int delayTime;

public:
    LEDBlink(gpio_num_t ledPin, int delayMs);
    void blink();
};

#endif // LEDBLINK_H