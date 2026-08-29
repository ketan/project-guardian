#pragma once

#include <Arduino.h>

class Tee : public Print {
public:
    static constexpr size_t maxDestinations = 4;

    bool add(Print &destination) {
        if (destinationCount == maxDestinations) {
            return false;
        }

        destinations[destinationCount++] = &destination;
        return true;
    }

    size_t write(uint8_t byte) override {
        return write(&byte, 1);
    }

    size_t write(const uint8_t *buffer, size_t size) override {
        size_t written = size;
        for (size_t index = 0; index < destinationCount; ++index) {
            const size_t destinationWritten = destinations[index]->write(buffer, size);
            if (destinationWritten < written) {
                written = destinationWritten;
            }
        }
        return written;
    }

private:
    Print *destinations[maxDestinations]{};
    size_t destinationCount = 0;
};
