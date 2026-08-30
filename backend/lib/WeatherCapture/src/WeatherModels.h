#pragma once

#include <cstdint>
#include <time.h>
#include <SEN0658.h>
#include <HttpJsonSerializable.h>

struct WindAggregate {
    float sustainedSpeedMetersPerSecond;
    float sustainedDirectionDegrees;
    float gustSpeedMetersPerSecond;
};

struct FiveMinuteWeatherAggregate {
    float temperatureCelsius;
    float humidityPercent;
    float pressureKiloPascals;
    float noiseDecibels;
    uint32_t illuminanceLux;
};

class WeatherRecord : public HttpJsonSerializable {
public:
    time_t recordedAt;
    WindAggregate wind;
    FiveMinuteWeatherAggregate stableWeather;
    SEN0658Readings latestSnapshot;
    bool hasRainfallLastHour;
    float rainfallLastHourMillimeters;
    float rainfallTotalMillimeters;

    bool hasTimestamp() const {
        return recordedAt >= 1704067200;
    }

    void toJSON(JsonObject json) const override {
        char timestamp[21];
        tm utc{};
        gmtime_r(&recordedAt, &utc);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &utc);
        json["recordedAt"] = timestamp;
        json["windSpeedMps"] = wind.sustainedSpeedMetersPerSecond;
        json["windGustMps"] = wind.gustSpeedMetersPerSecond;
        json["windDirectionDeg"] = wind.sustainedDirectionDegrees;
        json["temperatureC"] = stableWeather.temperatureCelsius;
        json["humidityPct"] = stableWeather.humidityPercent;
        json["pressureHpa"] = stableWeather.pressureKiloPascals * 10.0F;
        json["rainfallTotalMm"] = rainfallTotalMillimeters;
        json["illuminanceLux"] = stableWeather.illuminanceLux;
        json["pm2_5UgM3"] = latestSnapshot.pm25MicrogramsPerCubicMeter;
        json["pm10UgM3"] = latestSnapshot.pm10MicrogramsPerCubicMeter;
        json["noiseDb"] = stableWeather.noiseDecibels;
        if (hasRainfallLastHour) {
            json["rainfallLastHourMm"] = rainfallLastHourMillimeters;
        }
    }
};
