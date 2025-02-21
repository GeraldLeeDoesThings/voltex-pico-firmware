#pragma once
#include <optional>
#include "pico/stdlib.h"
#include "report.hpp"

#define JOYSTICK_SENSITIVITY 25
#define JOYSTICK_TIMEOUT_US 8000
#define MAX_JOYSTICKS 2

class Joystick {
private:
    alarm_pool_t* alarm_pool;
    std::optional<alarm_id_t> alarm;
    uint8_t z;
    uint8_t rotation_x;
    uint8_t rotation_y;
    uint8_t rotation_z;
    uint64_t last_update_time;

    Joystick(alarm_pool_t* alarm_pool);
    void cancel_alarm();
    void set_alarm();

public:
    void handle_encoder_left_rotation();
    void handle_encoder_right_rotation();
    void reset_position();
    void apply_to_report(report &report);
};

int64_t joystick_alarm_callback(alarm_id_t id, void *user_data);

