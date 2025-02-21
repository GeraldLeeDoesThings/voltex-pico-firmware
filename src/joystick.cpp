#include "joystick.hpp"

Joystick::Joystick(alarm_pool_t* alarm_pool):
    alarm_pool(alarm_pool),
    alarm(std::nullopt),
    z(0),
    rotation_x(0),
    rotation_y(0),
    rotation_z(0),
    last_update_time(0)
{}

void Joystick::cancel_alarm() {
    if (alarm.has_value()) {
        alarm_pool_cancel_alarm(alarm_pool, alarm.value());
        alarm = std::nullopt;
    }
}

void Joystick::set_alarm() {
    cancel_alarm();
    alarm = std::optional<alarm_id_t>{alarm_pool_add_alarm_in_us(alarm_pool, JOYSTICK_TIMEOUT_US, joystick_alarm_callback, this, true)};
}

void Joystick::handle_encoder_left_rotation() {
    rotation_x = (rotation_x < JOYSTICK_SENSITIVITY) ? 0 : rotation_x - JOYSTICK_SENSITIVITY;
    last_update_time = time_us_64();
    set_alarm();
}

void Joystick::handle_encoder_right_rotation() {
    rotation_x = (rotation_x > 0xFF - JOYSTICK_SENSITIVITY) ? 0xFF : rotation_x + JOYSTICK_SENSITIVITY;
    last_update_time = time_us_64();
    set_alarm();
}

void Joystick::reset_position() {
    rotation_x = 0;
    alarm = std::nullopt;
}

void Joystick::apply_to_report(report &report) {
    report.joystick_rotation_x = rotation_x;
}

int64_t joystick_alarm_callback(alarm_id_t id, void *user_data) {
    Joystick* joystick = static_cast<Joystick*>(user_data);
    joystick->reset_position();
    return 0;
}
