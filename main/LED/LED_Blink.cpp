#include "LED_Blink.h"

LEDBlink::LEDBlink(gpio_num_t ledPin, int delayMs) {
    pin = ledPin;
    delayTime = delayMs;
    esp_rom_gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

void LEDBlink::blink() {
    gpio_set_level(pin, 1);  // Turn LED ON
    vTaskDelay(pdMS_TO_TICKS(delayTime));
    gpio_set_level(pin, 0);  // Turn LED OFF
    vTaskDelay(pdMS_TO_TICKS(delayTime));
}