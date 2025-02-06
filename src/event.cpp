#include "event.hpp"

void record_event(uint gpio, uint32_t mask) {
    [[unlikely]] if (!EVENT_QUEUE.push(Event(gpio, mask))) {
        panic("Event buffer overflowed!");
    }
}

std::optional<Event> pop_event() {
    return EVENT_QUEUE.pop();
}
