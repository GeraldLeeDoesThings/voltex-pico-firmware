#include "rotary_encoder.hpp"

RotaryTransitionCounter::RotaryTransitionCounter():
    counts {0, 0, 0}
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
    event_buffer(ROTARY_ENCODER_EVENT_BUFFERS[num_rotary_encoders], ROTARY_ENCODER_EVENT_BUFFER_LEN),
    transition_buffer(ROTARY_ENCODER_TRANSITION_BUFFERS[num_rotary_encoders], ROTARY_ENCODER_DEBOUNCE_COUNT),
    last_state(UNKNOWN),
    transitions()
{
    if (++num_rotary_encoders > MAX_ROTARY_ENCODERS) [[unlikely]] {
        printf("Number of rotary encoders (%u) exceeds maximum (%u)!\n", num_rotary_encoders, MAX_ROTARY_ENCODERS);
        exit(1);
    }
    gpio_init(gpio_pin_left);
    gpio_init(gpio_pin_right);
    gpio_set_dir(gpio_pin_left, GPIO_IN);
    gpio_set_dir(gpio_pin_right, GPIO_IN);
    gpio_pull_up(gpio_pin_left);
    gpio_pull_up(gpio_pin_right);
    refresh_state();
}

bool RotaryEncoder::observe(RotaryEncoderEvent event) {
    return event_buffer.push(event);
}

void RotaryEncoder::handle_events() {
    std::optional<RotaryEncoderEvent> event = event_buffer.pop();
    while (event.has_value()) {
        if (handle_event(event.value())) [[likely]] {
            event = event_buffer.pop();
        }
        else [[unlikely]] {
            event_buffer.reset();
            refresh_state();
            break;
        }
    }
}

bool RotaryEncoder::handle_event(RotaryEncoderEvent event) {
    std::optional<RotaryEncoderState> next_state = std::nullopt;
    std::optional<RotaryEncoderTransition> transition = std::nullopt;
    switch (last_state) {
        case BOTH_DOWN:
            switch (event) {
                case LEFT_EDGE_RISE:
                    next_state.emplace(LEFT_UP);
                    transition.emplace(ROTATE_LEFT);
                    break;
                case RIGHT_EDGE_RISE:
                    next_state.emplace(RIGHT_UP);
                    transition.emplace(ROTATE_RIGHT);
                    break;
            }
            break;
        case LEFT_UP:
            switch (event) {
                case LEFT_EDGE_FALL:
                    next_state.emplace(BOTH_DOWN);
                    transition.emplace(ROTATE_RIGHT);
                    break;
                case RIGHT_EDGE_RISE:
                    next_state.emplace(BOTH_UP);
                    transition.emplace(ROTATE_LEFT);
                    break;
            }
            break;
        case RIGHT_UP:
            switch (event) {
                case LEFT_EDGE_RISE:
                    next_state.emplace(BOTH_UP);
                    transition.emplace(ROTATE_RIGHT);
                    break;
                case RIGHT_EDGE_FALL:
                    next_state.emplace(BOTH_DOWN);
                    transition.emplace(ROTATE_LEFT);
                    break;
            }
            break;
        case BOTH_UP:
            switch (event) {
                case LEFT_EDGE_FALL:
                    next_state.emplace(RIGHT_UP);
                    transition.emplace(ROTATE_LEFT);
                    break;
                case RIGHT_EDGE_FALL:
                    next_state.emplace(LEFT_UP);
                    transition.emplace(ROTATE_RIGHT);
                    break;
            }
            break;
        [[unlikely]] case UNKNOWN: 
            printf("Rotary encoder last known state is uninitialized!\n");
            exit(1);
    }

    if (next_state.has_value()) [[likely]] {
        last_state = next_state.value();
        std::optional<RotaryEncoderTransition> popped = transition_buffer.push(transition.value());
        transitions.observe(transition.value());
        switch (transition.value()) {
            case ROTATE_LEFT:
                printf("L");
                break;
            case ROTATE_RIGHT:
                printf("R");
                break;
        }
        if (popped.has_value()) {
            transitions.unobserve(popped.value());
        }
        return true;
    }
    else [[unlikely]] {
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
        printf("Attempted to create a Rotary Encoder handler before initializing statics\n");
        return false;
    }
    if (num_rotary_encoders < MAX_ROTARY_ENCODERS) {
        const uint index = num_rotary_encoders;
        ROTARY_ENCODERS[index] = RotaryEncoder(gpio_pin_left, gpio_pin_right);
        PIN_TO_ROTARY_ENCODER_HANDLER_MAP[gpio_pin_left] = index;
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

void run_rotary_encoder_tasks() {
    for (uint encoder_index = 0; encoder_index < MAX_ROTARY_ENCODERS; ++encoder_index) {
        std::optional<RotaryEncoder> encoder = ROTARY_ENCODERS[encoder_index];
        if (encoder.has_value()) {
            encoder.value().handle_events();
        }
    }
}

void handle_raw_rotary_encoder_irq(uint gpio, uint32_t event_mask) {
    std::optional<uint> rotary_encoder_index = PIN_TO_ROTARY_ENCODER_HANDLER_MAP[gpio];
    if (rotary_encoder_index.has_value()) {
        RotaryEncoder rotary_encoder = ROTARY_ENCODERS[rotary_encoder_index.value()].value();
        if (event_mask & GPIO_IRQ_EDGE_FALL) {
            if (gpio == rotary_encoder.get_left_pin()) {
                rotary_encoder.observe(LEFT_EDGE_FALL);
            }
            else if (gpio == rotary_encoder.get_right_pin()) {
                rotary_encoder.observe(RIGHT_EDGE_FALL);
            }
        }
        if (event_mask & GPIO_IRQ_EDGE_RISE) {
            if (gpio == rotary_encoder.get_left_pin()) {
                rotary_encoder.observe(LEFT_EDGE_RISE);
            }
            else if (gpio == rotary_encoder.get_right_pin()) {
                rotary_encoder.observe(RIGHT_EDGE_RISE);
            }
        }
    }
}
