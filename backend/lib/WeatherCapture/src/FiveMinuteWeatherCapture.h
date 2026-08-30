#pragma once

#include <cstddef>
#include <cmath>
#include <SEN0658.h>
#include <WeatherModels.h>

class FiveMinuteWeatherCapture {
public:
    // One reading per minute, averaged into each five-minute record.
    static constexpr size_t sampleCapacity = 5;

    void add(const SEN0658Readings &reading) {
        samples[next] = reading;
        next = (next + 1) % sampleCapacity;
        if (count < sampleCapacity) ++count;
    }

    bool aggregate(FiveMinuteWeatherAggregate &result) const {
        if (count < sampleCapacity) return false;

        float temperature = 0.0F, humidity = 0.0F, pressure = 0.0F, noise = 0.0F;
        uint64_t illuminance = 0;
        for (size_t offset = 0; offset < sampleCapacity; ++offset) {
            const SEN0658Readings &reading = latest(offset);
            temperature += reading.temperatureCelsius;
            humidity += reading.humidityPercent;
            pressure += reading.pressureKiloPascals;
            noise += reading.noiseDecibels;
            illuminance += reading.illuminanceLux;
        }
        result = {
            .temperatureCelsius = temperature / sampleCapacity,
            .humidityPercent = humidity / sampleCapacity,
            .pressureKiloPascals = pressure / sampleCapacity,
            .noiseDecibels = noise / sampleCapacity,
            .illuminanceLux = static_cast<uint32_t>(std::lround(static_cast<float>(illuminance) / sampleCapacity))
        };
        return true;
    }

private:
    const SEN0658Readings &latest(const size_t offset) const {
        return samples[(next + sampleCapacity - 1 - offset) % sampleCapacity];
    }

    SEN0658Readings samples[sampleCapacity]{};
    size_t next = 0;
    size_t count = 0;
};
