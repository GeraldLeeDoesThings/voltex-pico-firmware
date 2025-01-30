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

    // bool led_on = false;
    // bool last_a = false;
    // bool last_b = false;
    
    init_rotary_encoder_handling();

    if (!RotaryEncoder::create_and_register(ROTARY_0_GPIO_0, ROTARY_0_GPIO_1)) {
        printf("Failed to create Rotary Encoder handler!\n");
        exit(1);
    }

    gpio_set_irq_callback(&gpio_callback);

    while (true) {
	    // pico_set_led(led_on);
        run_rotary_encoder_tasks();
        /*
        bool read_a = gpio_get(ROTARY_0_GPIO_0);
        bool read_b = gpio_get(ROTARY_0_GPIO_1);
        if (read_a != last_a || read_b != last_b) {
            printf(
                "A: %s B: %s\n",
                read_a ? "1" : "0",
                read_b ? "1" : "0"
            );
        }
        last_a = read_a;
        last_b = read_b;
	    // led_on = !led_on;
	    // sleep_ms(250);
        */
    }

    return 0;
}
