#pragma once

#include <cmath>
#include <WeatherModels.h>

class TenMinuteWindCapture {
private:
    struct WindSample {
        uint16_t speedCmPerSecond;
        uint16_t directionCentiDegrees;
    };

    static_assert(sizeof(WindSample) == 4, "WindSample must stay compact");

public:
    static constexpr size_t sampleCapacity = 600;

    void add(const float speedMetersPerSecond, float directionDegrees) {
        const uint32_t speedCmPerSecond = speedMetersPerSecond > 0.0F
                                              ? static_cast<uint32_t>(std::lround(speedMetersPerSecond * 100.0F))
                                              : 0;
        directionDegrees = std::fmod(directionDegrees, 360.0F);
        if (directionDegrees < 0.0F) {
            directionDegrees += 360.0F;
        }
        const WindSample sample = {
            .speedCmPerSecond = static_cast<uint16_t>(speedCmPerSecond > UINT16_MAX ? UINT16_MAX : speedCmPerSecond),
            .directionCentiDegrees = static_cast<uint16_t>(
                static_cast<uint32_t>(std::lround(directionDegrees * 100.0F)) % 36000U),
        };
        samples[next] = sample;
        next = (next + 1) % sampleCapacity;
        if (count < sampleCapacity) {
            ++count;
        }
    }

    bool aggregate(WindAggregate &result) const {
        if (count < sustainedSamples) {
            return false;
        }

        float northSpeed = 0.0F;
        float eastSpeed = 0.0F;

        // Convert the latest two minutes of integer samples to m/s and degrees before
        // averaging their north/east vectors. This keeps directions around 0°/360° adjacent.
        for (size_t offset = 0; offset < sustainedSamples; ++offset) {
            const WindSample &sample = latest(offset);
            const float speedMetersPerSecond = sample.speedCmPerSecond / 100.0F;
            const float directionDegrees = sample.directionCentiDegrees / 100.0F;
            const float angleRadians = directionDegrees * radiansPerDegree;
            northSpeed += speedMetersPerSecond * std::cos(angleRadians);
            eastSpeed += speedMetersPerSecond * std::sin(angleRadians);
        }
        northSpeed /= sustainedSamples;
        eastSpeed /= sustainedSamples;

        result.sustainedSpeedMetersPerSecond = std::sqrt(northSpeed * northSpeed + eastSpeed * eastSpeed);
        float directionDegrees = std::atan2(eastSpeed, northSpeed) / radiansPerDegree;
        if (directionDegrees < 0.0F) {
            directionDegrees += 360.0F;
        }
        result.sustainedDirectionDegrees = directionDegrees;

        uint16_t gust = 0;
        // Scan the available ten-minute ring for the largest three-second speed mean.
        // Keep the sum in cm/s so the temporary cannot overflow and the stored gust stays exact.
        for (size_t offset = 0; offset + gustWindowSamples <= count; ++offset) {
            uint32_t sum = 0;
            for (size_t windowOffset = 0; windowOffset < gustWindowSamples; ++windowOffset) {
                sum += latest(offset + windowOffset).speedCmPerSecond;
            }
            const uint16_t average = static_cast<uint16_t>(sum / gustWindowSamples);
            if (average > gust) {
                gust = average;
            }
        }
        result.gustSpeedMetersPerSecond = gust / 100.0F;
        return true;
    }

    size_t size() const { return count; }

private:
    WindSample samples[sampleCapacity]{};
    size_t next = 0;
    size_t count = 0;

    static constexpr size_t sustainedSamples = 120;
    static constexpr size_t gustWindowSamples = 3;
    static constexpr float radiansPerDegree = 3.14159265358979323846F / 180.0F;

    const WindSample &latest(const size_t offset) const {
        return samples[(next + sampleCapacity - 1 - offset) % sampleCapacity];
    }
};

static_assert(sizeof(TenMinuteWindCapture) == 2400 + sizeof(size_t) * 2,
              "TenMinuteWindCapture must contain only the 2,400-byte sample ring and indexes");
