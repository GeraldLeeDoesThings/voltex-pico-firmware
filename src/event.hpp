#pragma once
#include <optional>
#include "pico/stdlib.h"
#include "buffer.hpp"

#define EVENT_BUFFER_LENGTH 2048

class Event {
public:
    uint gpio;
    uint32_t mask;
    uint64_t time;

    Event(uint gpio, uint32_t mask):
        gpio(gpio),
        mask(mask),
        time(time_us_64())
    {}

    Event():
        gpio(0),
        mask(0),
        time(0)
    {}

};

static Event EVENT_BUFFER[EVENT_BUFFER_LENGTH];
static CircularBufferFIFOQueue<Event> EVENT_QUEUE = CircularBufferFIFOQueue(EVENT_BUFFER, EVENT_BUFFER_LENGTH);

void record_event(uint gpio, uint32_t mask);
std::optional<Event> pop_event();
