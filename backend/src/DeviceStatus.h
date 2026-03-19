#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "HttpJsonSerializable.h"


class DeviceSummary : public HttpJsonSerializable {
public:
    String deviceId = "guardian-kamshet-01";
    String firmwareVersion = "0.1.0";
    String hardwareModel = "ESP32-S3 + SIM7670G + SEN0658";
    unsigned long uptimeSeconds = 0;
    String currentTime = "2026-03-19T08:21:53Z";
    String lastBootReason = "power_on";

    void toJSON(JsonObject json) const override;
};

class WifiStatus : public HttpJsonSerializable {
public:
    bool enabled = true;
    bool active = true;
    bool connected = true;
    String ssid = "guardian-station";
    String ipAddress = "192.168.4.1";
    int rssiDbm = -59;

    void toJSON(JsonObject json) const override;
};

class CellularStatus : public HttpJsonSerializable {
public:
    bool enabled = true;
    bool active = false;
    bool registered = true;
    String modemType = "SIM7670G";
    String operatorName = "Airtel";
    int signalQuality = 24;
    String ipv4 = "10.111.42.9";
    String ipv6 = "2409:4043:9c2:1::9";
    String imei;

    void toJSON(JsonObject json) const override;
};

class ConnectivityStatus : public HttpJsonSerializable {
public:
    WifiStatus wifi;
    CellularStatus cellular;

    void toJSON(JsonObject json) const override;
};

class StorageStatus : public HttpJsonSerializable {
public:
    bool sdCardPresent = false;
    uint64_t freeBytes = 0;
    uint64_t usedBytes = 0;
    int retentionDays = 14;
    String oldestRecordAt = "2026-03-01T00:00:00Z";
    String newestRecordAt = "2026-03-19T08:21:30Z";

    void toJSON(JsonObject json) const override;
};

class SamplingStatus : public HttpJsonSerializable {
public:
    int intervalSeconds = 10;
    String nextSampleAt = "2026-03-19T08:22:00Z";
    String lastSampleAt = "2026-03-19T08:21:50Z";
    bool sleepEnabled = true;
    bool smoothingEnabled = true;

    void toJSON(JsonObject json) const override;
};

class AdminWindowStatus : public HttpJsonSerializable {
public:
    bool active = false;
    String openedAt;
    String expiresAt;
    String requestedBy;

    void toJSON(JsonObject json) const override;
};

class SensorStatus : public HttpJsonSerializable {
public:
    String id;
    String kind;
    bool enabled = true;
    bool healthy = true;
    String lastReadAt = "2026-03-19T08:21:50Z";
    String message = "All metrics updating on schedule";

    void toJSON(JsonObject json) const override;
};

class PublisherStatus : public HttpJsonSerializable {
public:
    String type;
    bool enabled = false;
    String lastPublishAt;
    String lastResult = "unknown";
    String message;

    void toJSON(JsonObject json) const override;
};

class DeviceStatus : public HttpJsonSerializable {
public:
    DeviceSummary device;
    ConnectivityStatus connectivity;
    StorageStatus storage;
    SamplingStatus sampling;
    AdminWindowStatus adminWindow;
    std::vector<SensorStatus> sensors;
    std::vector<PublisherStatus> publishers;

    void toJSON(JsonObject json) const override;
};

class WeatherSample : public HttpJsonSerializable {
public:
    String recordedAt = "2026-03-19T08:21:50Z";
    double temperatureC = 24.2;
    double humidityPct = 47.5;
    double pressureHpa = 1008.4;
    double windSpeedMps = 5.3;
    double windGustMps = 7.1;
    double windDirectionDeg = 245.0;
    double rainfallMm = 0.0;
    double illuminanceLux = 18450.0;

    void toJSON(JsonObject json) const override {
        return toJSON(json, false);
    }

    void toJSON(JsonObject json, bool includeSmoothingApplied) const;

    static void writeArray(JsonArray array, const std::vector<WeatherSample> &history);
};

class LatestSensorReadings : public HttpJsonSerializable {
public:
    WeatherSample latest;
    WeatherSample smoothed;
    bool smoothingApplied = true;

    void toJSON(JsonObject json) const override;
};
