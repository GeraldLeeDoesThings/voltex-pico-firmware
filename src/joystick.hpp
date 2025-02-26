#pragma once
#include <optional>
#include <stdio.h>
#include "pico/stdlib.h"
#include "buffer.hpp"
#include "report.hpp"

#define JOYSTICK_SENSITIVITY 1
#define MAX_JOYSTICKS 2

class Joystick {
private:
    static uint num_joysticks;
    uint8_t z;
    uint8_t rotation_x;
    uint8_t rotation_y;
    uint8_t rotation_z;
    bool changed;

    Joystick();

public:
    static std::optional<Joystick*> create_and_register();
    void handle_encoder_left_rotation();
    void handle_encoder_right_rotation();
    void apply_to_report(report &report);
    bool has_changes();
};

static std::optional<Joystick> JOYSTICKS[MAX_JOYSTICKS];
