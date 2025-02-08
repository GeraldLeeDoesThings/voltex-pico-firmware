#include "button.hpp"

Button::Button(uint pin):
    pressed(false),
    last_update(0),
    gpio_pin(pin)
{
    if (++num_buttons > MAX_BUTTONS) [[unlikely]] {
        panic("Number of buttons (%u) exceeds maximum (%u)!\n", num_buttons, MAX_BUTTONS);
    }
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_down(pin);
    refresh_state();
}

void Button::refresh_state() {
    pressed = gpio_get(gpio_pin);
    last_update = time_us_64();
}

bool Button::create_and_register(uint pin) {
    if (!BUTTON_STATICS_INITIALIZED) {
        panic("Attempted to create a Button handler before intializing statics!\n");
    }
    if (num_buttons < MAX_BUTTONS) {
        if (PIN_TO_BUTTON_HANDLER_MAP[pin].has_value()) {
            return false;
        }
        const uint index = num_buttons;
        BUTTONS[index] = Button(pin);
        PIN_TO_BUTTON_HANDLER_MAP[pin] = index;
        return true;
    }
    return false;
}

void Button::handle_event(const TimedButtonEvent &event) {
    std::optional<bool> is_now_pressed = std::nullopt;
    switch (event.event) {
        case BUTTON_UP:
            if (pressed) {
                is_now_pressed = false;
            }
            break;
        case BUTTON_DOWN:
            if (!pressed) {
                is_now_pressed = true;
            }
            break;
    }
    if (is_now_pressed.has_value()) {
        pressed = is_now_pressed.value();
        if (pressed) {
            printf("D\n");
        }
        else {
            printf("U\n");
        }
    }
}

uint Button::get_pin() {
    return gpio_pin;
}

void init_button_handling() {
    for (uint button = 0; button < MAX_BUTTONS; ++button) {
        BUTTONS[button] = std::nullopt;
    }
    for (uint pin = 0; pin < MAX_GPIO_PINS; ++pin) {
        PIN_TO_BUTTON_HANDLER_MAP[pin] = std::nullopt;
    }
    BUTTON_STATICS_INITIALIZED = true;
}

void handle_button_event(const Event &event) {
    uint gpio = event.gpio;
    uint32_t event_mask = event.mask;
    uint64_t at = event.time;
    std::optional<uint> button_index = PIN_TO_BUTTON_HANDLER_MAP[gpio];
    if (button_index.has_value()) {
        std::optional<Button>& button = BUTTONS[button_index.value()];
        if (button.has_value()) {
            bool edge_fall = event_mask & GPIO_IRQ_EDGE_FALL;
            bool edge_rise = event_mask & GPIO_IRQ_EDGE_RISE;
            if (edge_rise && edge_fall) {
                // Do nothing...
            }
            else if (edge_fall) {
                button.value().handle_event(TimedButtonEvent { BUTTON_UP, at });
            }
            else if (edge_rise) {
                button.value().handle_event(TimedButtonEvent { BUTTON_DOWN, at });
            }
            
        }
    }
}

void enable_button_irq() {
    for (uint button_index = 0; button_index < MAX_BUTTONS; ++button_index) {
        std::optional<Button>& button = BUTTONS[button_index];
        if (button.has_value()) {
            gpio_set_irq_enabled(button.value().get_pin(), GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
        }
    }
}

uint Button::num_buttons = 0;
