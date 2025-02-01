#pragma once
#include "pico/stdlib.h"
#include <optional>

template <typename T> 
class CircularBufferFIFOQueue {
private:
    T* data;
    volatile uint len;
    uint size;
    volatile uint write_index;
    volatile uint read_index;
public:
    CircularBufferFIFOQueue(T* data, uint size):
        data(data),
        len(0),
        size(size),
        write_index(0),
        read_index(0)
    { }

    bool push(T val) {
        if (len == size) {
            return false;
        }
        data[write_index++] = val;
        write_index = write_index % size;
        ++len;
        return true;
    }

    std::optional<T> pop() {
        if (len == 0) {
            return std::nullopt;
        }
        T val = data[read_index++];
        read_index = read_index % size;
        --len;
        return std::optional<T>{val};
    }

    void reset() {
        read_index = write_index;
        len = 0;
    }

    uint get_len() {
        return len;
    }
};

template <typename T> 
class CircularOverflowBuffer {
private:
    std::optional<T>* data;
    uint size;
    volatile uint head;
public:
    CircularOverflowBuffer(std::optional<T>* data, uint size):
        data(data),
        size(size),
        head(0)
    {
        for (uint i = 0; i < size; ++i) {
            data[i] = std::nullopt;
        }
    }

    std::optional<T> push(T val) {
        std::optional<T> result = std::optional<T>{val};
        data[head++].swap(result);
        head = head % size;
        return result;
    }
};
