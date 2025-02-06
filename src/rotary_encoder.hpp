#pragma once
#include <optional>
#include <stdio.h>
#include "pico/stdlib.h"
#include "buffer.hpp"
#include "const.hpp"
#include "event.hpp"
#define ROTARY_ENCODER_EVENT_BUFFER_LEN 256
#define ROTARY_ENCODER_DEBOUNCE_COUNT 5
#define ROTARY_ENCODER_CONSENSUS_COUNT 3
#define MAX_ROTARY_ENCODERS 2
#define MIN_US_DIFF_TO_SEND 8000llu

enum RotaryEncoderState {
    BOTH_DOWN,
    LEFT_UP,
    RIGHT_UP,
    BOTH_UP,
    UNKNOWN,
};

enum RotaryEncoderEvent {
    LEFT_EDGE_FALL,
    LEFT_EDGE_RISE,
    RIGHT_EDGE_FALL,
    RIGHT_EDGE_RISE,
};

struct TimedRotaryEncoderEvent {
    RotaryEncoderEvent event;
    uint64_t time;
};

enum RotaryEncoderTransition {
    ROTATE_LEFT = 0,
    ROTATE_RIGHT = 1,
};

class RotaryTransitionCounter {
    friend class RotaryEncoder;
private:
    uint counts[2];
    RotaryTransitionCounter();
    void observe(RotaryEncoderTransition transition);
    bool unobserve(RotaryEncoderTransition transition);
    uint count(RotaryEncoderTransition transition);
};

class RotaryEncoder {
private:
    static uint num_rotary_encoders;
    uint gpio_pin_left;
    uint gpio_pin_right;
    CircularOverflowBuffer<RotaryEncoderTransition> transition_buffer;
    RotaryEncoderState last_state;
    RotaryTransitionCounter transitions;
    uint64_t last_state_update;
    bool last_read_ok;

    RotaryEncoder(uint gpio_pin_left, uint gpio_pin_right);
    void refresh_state();

public:
    
    static bool create_and_register(uint gpio_pin_left, uint gpio_pin_right);
    bool handle_event(const TimedRotaryEncoderEvent &event);
    uint get_left_pin();
    uint get_right_pin();
};

static std::optional<RotaryEncoderTransition> ROTARY_ENCODER_TRANSITION_BUFFERS[MAX_ROTARY_ENCODERS][ROTARY_ENCODER_DEBOUNCE_COUNT];
static std::optional<RotaryEncoder> ROTARY_ENCODERS[MAX_ROTARY_ENCODERS];
static std::optional<uint> PIN_TO_ROTARY_ENCODER_HANDLER_MAP[MAX_GPIO_PINS];
static bool ROTARY_ENCODER_STATICS_INITIALIZED = false;

void init_rotary_encoder_handling();
void handle_rotary_encoder_event(const Event &event);
void enable_rotary_encoder_irq();
