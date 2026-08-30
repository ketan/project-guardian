#pragma once

#include <Arduino.h>
#include <Logger.h>
#include <SEN0658.h>

#include "FiveMinuteWeatherCapture.h"
#include "OneHourRainfallCapture.h"
#include "TenMinuteWindCapture.h"

class WeatherCapture {
public:
    bool capture(SEN0658 &sensor, WeatherRecord &record) {
        const uint32_t now = millis();
        if (!isDue(now, lastWindCaptureMilliseconds, hasWindCapture, windIntervalMilliseconds)) {
            return false;
        }

        lastWindCaptureMilliseconds = now;
        hasWindCapture = true;
        float speedMetersPerSecond;
        uint16_t directionDegrees;
        if (!sensor.readWind(speedMetersPerSecond, directionDegrees)) {
            WARN("Wind poll: failed");
            return false;
        }
        windCapture.add(speedMetersPerSecond, directionDegrees);

        if (isDue(now, lastSnapshotCaptureMilliseconds, hasSnapshotCapture, snapshotIntervalMilliseconds)) {
            lastSnapshotCaptureMilliseconds = now;
            hasSnapshotCapture = true;
            if (!sensor.readLatestSnapshot(latestSnapshot)) {
                WARN("Stable weather poll: failed");
                return false;
            }
            fiveMinuteWeatherCapture.add(latestSnapshot);
            rainfallCapture.add(latestSnapshot.rainfallMillimeters);
            hasLatestSnapshot = true;
        }

        if (!isDue(now, lastAggregateMilliseconds, hasAggregate, aggregateIntervalMilliseconds)) {
            return false;
        }
        lastAggregateMilliseconds = now;
        hasAggregate = true;

        if (!windCapture.aggregate(record.wind) || !fiveMinuteWeatherCapture.aggregate(record.stableWeather)) {
            WARN("Weather aggregate: waiting for stable readings");
            return false;
        }
        if (!hasLatestSnapshot) {
            WARN("Weather aggregate: waiting for sensor snapshot");
            return false;
        }
        record.recordedAt = time(nullptr);
        record.latestSnapshot = latestSnapshot;
        record.hasRainfallLastHour = rainfallCapture.lastHour(record.rainfallLastHourMillimeters);
        rainfallCapture.latest(record.rainfallTotalMillimeters);
        return true;
    }

private:
    static constexpr uint32_t windIntervalMilliseconds = 1000;
    static constexpr uint32_t snapshotIntervalMilliseconds = 60000;
    static constexpr uint32_t aggregateIntervalMilliseconds = 300000;

    static bool isDue(const uint32_t now, const uint32_t lastCapture, const bool hasCaptured,
                      const uint32_t intervalMilliseconds) {
        return !hasCaptured || now - lastCapture >= intervalMilliseconds;
    }

    TenMinuteWindCapture windCapture;
    FiveMinuteWeatherCapture fiveMinuteWeatherCapture;
    OneHourRainfallCapture rainfallCapture;
    SEN0658Readings latestSnapshot{};
    uint32_t lastWindCaptureMilliseconds = 0;
    uint32_t lastSnapshotCaptureMilliseconds = 0;
    uint32_t lastAggregateMilliseconds = 0;
    bool hasWindCapture = false;
    bool hasSnapshotCapture = false;
    bool hasAggregate = false;
    bool hasLatestSnapshot = false;
};
