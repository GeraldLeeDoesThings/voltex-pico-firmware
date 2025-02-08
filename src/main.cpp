#include <optional>
#include <stdio.h>
#include "pico/stdlib.h"
#include "button.hpp"
#include "event.hpp"
#include "rotary_encoder.hpp"

#define ROTARY_0_GPIO_0 0
#define ROTARY_0_GPIO_1 1
#define BUTTON_0_GPIO  16

void pico_led_init() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void pico_set_led(bool on) {
    gpio_put(PICO_DEFAULT_LED_PIN, on);
}

void gpio_callback(uint gpio, uint32_t event_mask) {
    record_event(gpio, event_mask);
    // irq is automatically acknowledged
}

int main() {
    stdio_init_all();
    pico_led_init();

    sleep_ms(3000);  // LOAD BEARING!!

    printf("Ready!\n");
    
    init_rotary_encoder_handling();
    init_button_handling();

    if (!RotaryEncoder::create_and_register(ROTARY_0_GPIO_0, ROTARY_0_GPIO_1)) {
        panic("Failed to create Rotary Encoder handler!\n");
    }

    if (!Button::create_and_register(BUTTON_0_GPIO)) {
        panic("Failed to create Button handler!\n");
    }

    pico_set_led(true);

    gpio_set_irq_callback(&gpio_callback);
    enable_rotary_encoder_irq();
    enable_button_irq();
    irq_set_enabled(IO_IRQ_BANK0, true);

    while (true) {

        std::optional<Event> maybe_event = pop_event();
        while (maybe_event.has_value()) {
            handle_rotary_encoder_event(maybe_event.value());
            handle_button_event(maybe_event.value());
            maybe_event = pop_event();
        }
    }

    return 0;
}
