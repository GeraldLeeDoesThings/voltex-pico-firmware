#include <stdio.h>
#include "pico/stdlib.h"
#include "rotary_encoder.hpp"

#define ROTARY_0_GPIO_0 0
#define ROTARY_0_GPIO_1 1

void pico_led_init() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void pico_set_led(bool on) {
    gpio_put(PICO_DEFAULT_LED_PIN, on);
}

void gpio_callback(uint gpio, uint32_t event_mask) {
    handle_raw_rotary_encoder_irq(gpio, event_mask);
    // irq is automatically acknowledged
}

int main() {
    stdio_init_all();
    pico_led_init();

    sleep_ms(3000);  // LOAD BEARING!!

    printf("Ready!\n");
    
    init_rotary_encoder_handling();

    if (!RotaryEncoder::create_and_register(ROTARY_0_GPIO_0, ROTARY_0_GPIO_1)) {
        panic("Failed to create Rotary Encoder handler!\n");
    }

    pico_set_led(true);

    gpio_set_irq_callback(&gpio_callback);
    enable_rotary_encoder_irq();
    irq_set_enabled(IO_IRQ_BANK0, true);

    while (true) {
        run_rotary_encoder_tasks();
    }

    return 0;
}
