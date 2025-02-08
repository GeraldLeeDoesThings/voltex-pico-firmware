#pragma once
#include <optional>
#include <stdio.h>
#include "pico/stdlib.h"
#include "const.hpp"
#include "event.hpp"

#define MAX_BUTTONS 7

enum ButtonEventType {
    BUTTON_UP,
    BUTTON_DOWN,
};

struct TimedButtonEvent {
    ButtonEventType event;
    uint64_t time;
};

class Button {
private:
    static uint num_buttons;
    bool pressed;
    uint64_t last_update;
    uint gpio_pin;

    Button(uint pin);
    void refresh_state();

public:
    static bool create_and_register(uint pin);
    void handle_event(const TimedButtonEvent &event);
    uint get_pin();
};

static std::optional<Button> BUTTONS[MAX_BUTTONS];
static std::optional<uint> PIN_TO_BUTTON_HANDLER_MAP[MAX_GPIO_PINS];
static bool BUTTON_STATICS_INITIALIZED = false;

void init_button_handling();
void handle_button_event(const Event &event);
void enable_button_irq();
