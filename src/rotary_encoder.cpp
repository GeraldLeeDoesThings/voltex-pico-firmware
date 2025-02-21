#include "rotary_encoder.hpp"

RotaryTransitionCounter::RotaryTransitionCounter():
    counts {0, 0}
{ }

void RotaryTransitionCounter::observe(RotaryEncoderTransition transition) {
    ++counts[transition];
}

bool RotaryTransitionCounter::unobserve(RotaryEncoderTransition transition) {
    if (counts[transition] == 0) [[unlikely]] {
        return false;
    }
    else [[likely]] {
        --counts[transition];
        return true;
    }
}

uint RotaryTransitionCounter::count(RotaryEncoderTransition transition) {
    return counts[transition];
}

RotaryEncoder::RotaryEncoder(uint gpio_pin_left, uint gpio_pin_right):
    gpio_pin_left(gpio_pin_left),
    gpio_pin_right(gpio_pin_right),
    transition_buffer(ROTARY_ENCODER_TRANSITION_BUFFERS[num_rotary_encoders], ROTARY_ENCODER_DEBOUNCE_COUNT),
    last_state(UNKNOWN),
    transitions(),
    last_state_update(0),
    last_read_ok(true)
{
    if (++num_rotary_encoders > MAX_ROTARY_ENCODERS) [[unlikely]] {
        panic("Number of rotary encoders (%u) exceeds maximum (%u)!\n", num_rotary_encoders, MAX_ROTARY_ENCODERS);
    }
    gpio_init(gpio_pin_left);
    gpio_init(gpio_pin_right);
    gpio_set_dir(gpio_pin_left, GPIO_IN);
    gpio_set_dir(gpio_pin_right, GPIO_IN);
    // gpio_pull_up(gpio_pin_left);
    // gpio_pull_up(gpio_pin_right);
    // gpio_set_slew_rate(gpio_pin_left, GPIO_SLEW_RATE_SLOW);
    // gpio_set_slew_rate(gpio_pin_right, GPIO_SLEW_RATE_SLOW);
    refresh_state();
}

bool RotaryEncoder::handle_event(const TimedRotaryEncoderEvent &event) {
    std::optional<RotaryEncoderState> next_state = std::nullopt;
    std::optional<RotaryEncoderTransition> transition = std::nullopt;
    uint64_t now = event.time;
    uint64_t diff = now - last_state_update;
    bool fast = diff < MIN_US_DIFF_TO_SEND;
    bool very_fast = diff < MIN_US_DIFF_TO_SEND / 100;
    switch (last_state) {
        case BOTH_DOWN:
            switch (event.event) {
                case LEFT_EDGE_RISE:
                    next_state.emplace(LEFT_UP);
                    // Ambiguous
                    break;
                case RIGHT_EDGE_RISE:
                    next_state.emplace(RIGHT_UP);
                    transition.emplace(ROTATE_LEFT);
                    break;
            }
            break;
        case LEFT_UP:
            switch (event.event) {
                case LEFT_EDGE_FALL:
                    next_state.emplace(BOTH_DOWN);
                    transition.emplace((very_fast) ? ROTATE_RIGHT : ROTATE_LEFT);
                    break;
                case RIGHT_EDGE_RISE:
                    next_state.emplace(BOTH_UP);
                    transition.emplace((very_fast) ? ROTATE_LEFT : ROTATE_RIGHT);
                    break;
            }
            break;
        case RIGHT_UP:
            switch (event.event) {
                case LEFT_EDGE_RISE:
                    next_state.emplace(BOTH_UP);
                    transition.emplace(ROTATE_LEFT);
                    break;
                case RIGHT_EDGE_FALL:
                    next_state.emplace(BOTH_DOWN);
                    transition.emplace(ROTATE_RIGHT);
                    break;
            }
            break;
        case BOTH_UP:
            switch (event.event) {
                case LEFT_EDGE_FALL:
                    next_state.emplace(RIGHT_UP);
                    transition.emplace(ROTATE_RIGHT);
                    break;
                case RIGHT_EDGE_FALL:
                    next_state.emplace(LEFT_UP);
                    // Ambiguous
                    break;
            }
            break;
        [[unlikely]] case UNKNOWN: 
            panic("Rotary encoder last known state is uninitialized!\n");
    }

    if (next_state.has_value()) [[likely]] {
        last_state_update = now;
        last_state = next_state.value();
        if (!last_read_ok) {
            last_read_ok = true;
            return true;  // Last read was an error
        }
        if (fast) {
            return true;  // Too fast to send
        }
        if (transition.has_value()) {
            std::optional<RotaryEncoderTransition> popped = transition_buffer.push(transition.value());
            transitions.observe(transition.value());
            if (popped.has_value()) {
                transitions.unobserve(popped.value());
            }
            if (transitions.count(transition.value()) >= ROTARY_ENCODER_CONSENSUS_COUNT) {
                switch (transition.value()) {
                    case ROTATE_LEFT:
                        #ifdef DEBUG_MODE
                        printf("L\n");
                        #endif
                        break;
                    case ROTATE_RIGHT:
                        #ifdef DEBUG_MODE
                        printf("R\n");
                        #endif
                        break;
                }
            }
        }
        return true;
    }
    else [[unlikely]] {
        last_read_ok = false;
        #ifdef DEBUG_MODE
        printf("r");
        #endif
        return false;
    }
}

void RotaryEncoder::refresh_state() {
    if (gpio_get(gpio_pin_left)) {
        if (gpio_get(gpio_pin_right)) {
            last_state = BOTH_UP;
        }
        else {
            last_state = LEFT_UP;
        }
    }
    else {
        if (gpio_get(gpio_pin_right)) {
            last_state = RIGHT_UP;
        }
        else {
            last_state = BOTH_DOWN;
        }
    }
}

bool RotaryEncoder::create_and_register(uint gpio_pin_left, uint gpio_pin_right) {
    if (!ROTARY_ENCODER_STATICS_INITIALIZED) {
        panic("Attempted to create a Rotary Encoder handler before initializing statics\n");
    }
    if (num_rotary_encoders < MAX_ROTARY_ENCODERS) {
        if (PIN_TO_ROTARY_ENCODER_HANDLER_MAP[gpio_pin_left].has_value()) {
            return false;
        }
        if (PIN_TO_ROTARY_ENCODER_HANDLER_MAP[gpio_pin_right].has_value()) {
            return false;
        }
        const uint index = num_rotary_encoders;
        ROTARY_ENCODERS[index] = RotaryEncoder(gpio_pin_left, gpio_pin_right);
        PIN_TO_ROTARY_ENCODER_HANDLER_MAP[gpio_pin_left] = index;
        PIN_TO_ROTARY_ENCODER_HANDLER_MAP[gpio_pin_right] = index;
        return true;
    }
    return false;
}

uint RotaryEncoder::get_left_pin() {
    return gpio_pin_left;
}

uint RotaryEncoder::get_right_pin() {
    return gpio_pin_right;
}

uint RotaryEncoder::num_rotary_encoders = 0;

void init_rotary_encoder_handling() {
    for (uint encoder = 0; encoder < MAX_ROTARY_ENCODERS; ++encoder) {
        for (uint debounce_index = 0; debounce_index < ROTARY_ENCODER_EVENT_BUFFER_LEN; ++debounce_index) {
            ROTARY_ENCODER_TRANSITION_BUFFERS[encoder][debounce_index] = std::nullopt;
        }
        ROTARY_ENCODERS[encoder] = std::nullopt;
    }

    for (uint pin = 0; pin < MAX_GPIO_PINS; ++pin) {
        PIN_TO_ROTARY_ENCODER_HANDLER_MAP[pin] = std::nullopt;
    }
    ROTARY_ENCODER_STATICS_INITIALIZED = true;
}

void handle_rotary_encoder_event(const Event &event) {
    uint gpio = event.gpio;
    uint32_t event_mask = event.mask;
    uint64_t at = event.time;
    std::optional<uint> rotary_encoder_index = PIN_TO_ROTARY_ENCODER_HANDLER_MAP[gpio];
    if (rotary_encoder_index.has_value()) {
        RotaryEncoder& rotary_encoder = ROTARY_ENCODERS[rotary_encoder_index.value()].value();
        bool edge_fall = event_mask & GPIO_IRQ_EDGE_FALL;
        bool edge_rise = event_mask & GPIO_IRQ_EDGE_RISE;
        if (edge_fall && edge_rise) {
            // do nothing...
        }
        else if (edge_fall) {
            if (gpio == rotary_encoder.get_left_pin()) {
                rotary_encoder.handle_event(TimedRotaryEncoderEvent { LEFT_EDGE_FALL, at });
            }
            else if (gpio == rotary_encoder.get_right_pin()) {
                rotary_encoder.handle_event(TimedRotaryEncoderEvent { RIGHT_EDGE_FALL, at });
            }
            else {
                panic("gpio pin %u is mapped to rotary encoder %u, but neither of its pins match!", gpio, rotary_encoder_index.value());
            }
        }
        else if (edge_rise) {
            if (gpio == rotary_encoder.get_left_pin()) {
                rotary_encoder.handle_event(TimedRotaryEncoderEvent { LEFT_EDGE_RISE, at });
            }
            else if (gpio == rotary_encoder.get_right_pin()) {
                rotary_encoder.handle_event(TimedRotaryEncoderEvent { RIGHT_EDGE_RISE, at });
            }
            else {
                panic("gpio pin %u is mapped to rotary encoder %u, but neither of its pins match!", gpio, rotary_encoder_index.value());
            }
        }
    }
}

void enable_rotary_encoder_irq() {
    for (uint encoder_index = 0; encoder_index < MAX_ROTARY_ENCODERS; ++encoder_index) {
        std::optional<RotaryEncoder>& encoder = ROTARY_ENCODERS[encoder_index];
        if (encoder.has_value()) {
            gpio_set_irq_enabled(encoder.value().get_left_pin(), GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
            gpio_set_irq_enabled(encoder.value().get_right_pin(), GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
        }
    }
}
