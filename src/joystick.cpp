#include "joystick.hpp"

uint Joystick::num_joysticks = 0;

Joystick::Joystick():
    z(0),
    rotation_x(0),
    rotation_y(0),
    rotation_z(0),
    changed(false)
{
    if (num_joysticks >= MAX_JOYSTICKS) {
        panic("Tried to create a joystick, but the maximum number of joysticks already exist!");
    }
    num_joysticks;
}

std::optional<Joystick*> Joystick::create_and_register() {
    if (num_joysticks >= MAX_JOYSTICKS) {
        return std::nullopt;
    }
    JOYSTICKS[num_joysticks] = Joystick();
    return std::optional<Joystick*>{&JOYSTICKS[num_joysticks++].value()};
}

void Joystick::handle_encoder_left_rotation() {
    rotation_x = (rotation_x < JOYSTICK_SENSITIVITY) ? 0xFF - (JOYSTICK_SENSITIVITY - rotation_x) : rotation_x - JOYSTICK_SENSITIVITY;
    changed = true;
}

void Joystick::handle_encoder_right_rotation() {
    rotation_x = (rotation_x > 0xFF - JOYSTICK_SENSITIVITY) ? JOYSTICK_SENSITIVITY - (0xFF - rotation_x) : rotation_x + JOYSTICK_SENSITIVITY;
    changed = true;
}

void Joystick::apply_to_report(report &report) {
    report.joystick_rotation_x = rotation_x;
    changed = false;
}

bool Joystick::has_changes() {
    return changed;
}
