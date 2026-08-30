#pragma once

class OneHourRainfallCapture {
public:
    // A 60-minute delta needs both endpoints: now and 60 minutes ago.
    static constexpr size_t sampleCapacity = 61;

    void add(const float cumulativeMillimeters) {
        samples[next] = cumulativeMillimeters;
        next = (next + 1) % sampleCapacity;
        if (count < sampleCapacity) ++count;
    }

    bool lastHour(float &millimeters) const {
        if (count < sampleCapacity) return false;
        const float latest = samples[(next + sampleCapacity - 1) % sampleCapacity];
        const float oldest = samples[next];
        millimeters = latest >= oldest ? latest - oldest : 0.0F;
        return true;
    }

    bool latest(float &millimeters) const {
        if (count == 0) return false;
        millimeters = samples[(next + sampleCapacity - 1) % sampleCapacity];
        return true;
    }

private:
    float samples[sampleCapacity]{};
    size_t next = 0;
    size_t count = 0;
};
