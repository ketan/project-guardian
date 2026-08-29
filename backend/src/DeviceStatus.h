#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "HttpJsonSerializable.h"
#include "Version.h"


class DeviceSummary : public HttpJsonSerializable {
public:
    String deviceId = "guardian-kamshet-01";
    String firmwareVersion = VERSION;
    String hardwareModel = "ESP32-S3 + SIM7670G + SEN0658";
    unsigned long uptimeSeconds = 0;
    String currentTime = "2026-03-19T08:21:53Z";
    String lastBootReason = "power_on";

    void toJSON(JsonObject json) const override {
        json["deviceId"] = deviceId;
        json["firmwareVersion"] = firmwareVersion;
        json["hardwareModel"] = hardwareModel;
        json["uptimeSeconds"] = uptimeSeconds;
        json["currentTime"] = currentTime;
        json["lastBootReason"] = lastBootReason;
    }
};

class WifiStatus : public HttpJsonSerializable {
public:
    bool enabled = true;
    bool active = true;
    bool connected = true;
    String ssid = "guardian-station";
    String ipAddress = "192.168.4.1";
    int rssiDbm = -59;

    void toJSON(JsonObject json) const override {
        json["enabled"] = enabled;
        json["active"] = active;
        json["connected"] = connected;
        json["ssid"] = ssid;
        json["ipAddress"] = ipAddress;
        json["rssiDbm"] = rssiDbm;
    }
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

    void toJSON(JsonObject json) const override {
        json["enabled"] = enabled;
        json["active"] = active;
        json["registered"] = registered;
        json["modemType"] = modemType;
        json["operatorName"] = operatorName;
        json["signalQuality"] = signalQuality;
        json["ipv4"] = ipv4;
        json["ipv6"] = ipv6;
    }
};

class ConnectivityStatus : public HttpJsonSerializable {
public:
    WifiStatus wifi;
    CellularStatus cellular;

    void toJSON(JsonObject json) const override {
        wifi.toJSON(json["wifi"].to<JsonObject>());
        cellular.toJSON(json["cellular"].to<JsonObject>());
    }
};

class StorageStatus : public HttpJsonSerializable {
public:
    bool sdCardPresent = false;
    uint64_t freeBytes = 0;
    uint64_t usedBytes = 0;
    int retentionDays = 14;
    String oldestRecordAt = "2026-03-01T00:00:00Z";
    String newestRecordAt = "2026-03-19T08:21:30Z";

    void toJSON(JsonObject json) const override {
        json["sdCardPresent"] = sdCardPresent;
        json["freeBytes"] = freeBytes;
        json["usedBytes"] = usedBytes;
        json["retentionDays"] = retentionDays;
        json["oldestRecordAt"] = oldestRecordAt;
        json["newestRecordAt"] = newestRecordAt;
    }
};

class SamplingStatus : public HttpJsonSerializable {
public:
    int intervalSeconds = 10;
    String nextSampleAt = "2026-03-19T08:22:00Z";
    String lastSampleAt = "2026-03-19T08:21:50Z";
    bool sleepEnabled = true;
    bool smoothingEnabled = true;

    void toJSON(JsonObject json) const override {
        json["intervalSeconds"] = intervalSeconds;
        json["nextSampleAt"] = nextSampleAt;
        json["lastSampleAt"] = lastSampleAt;
        json["sleepEnabled"] = sleepEnabled;
        json["smoothingEnabled"] = smoothingEnabled;
    }
};

class AdminWindowStatus : public HttpJsonSerializable {
public:
    bool active = false;
    String openedAt;
    String expiresAt;
    String requestedBy;

    void toJSON(JsonObject json) const override {
        json["active"] = active;
        if (!openedAt.isEmpty()) json["openedAt"] = openedAt;
        if (!expiresAt.isEmpty()) json["expiresAt"] = expiresAt;
        if (!requestedBy.isEmpty()) json["requestedBy"] = requestedBy;
    }
};

class SensorStatus : public HttpJsonSerializable {
public:
    String id;
    String kind;
    bool enabled = true;
    bool healthy = true;
    String lastReadAt = "2026-03-19T08:21:50Z";
    String message = "All metrics updating on schedule";

    void toJSON(JsonObject json) const override {
        json["id"] = id;
        json["kind"] = kind;
        json["enabled"] = enabled;
        json["healthy"] = healthy;
        json["lastReadAt"] = lastReadAt;
        json["message"] = message;
    }
};

class PublisherStatus : public HttpJsonSerializable {
public:
    String type;
    bool enabled = false;
    String lastPublishAt;
    String lastResult = "unknown";
    String message;

    void toJSON(JsonObject json) const override {
        json["type"] = type;
        json["enabled"] = enabled;
        if (!lastPublishAt.isEmpty()) json["lastPublishAt"] = lastPublishAt;
        json["lastResult"] = lastResult;
        json["message"] = message;
    }
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

    void toJSON(JsonObject json) const override {
        device.toJSON(json["device"].to<JsonObject>());
        connectivity.toJSON(json["connectivity"].to<JsonObject>());
        storage.toJSON(json["storage"].to<JsonObject>());
        sampling.toJSON(json["sampling"].to<JsonObject>());
        adminWindow.toJSON(json["adminWindow"].to<JsonObject>());
        JsonArray sensorsJson = json["sensors"].to<JsonArray>();
        for (const SensorStatus &sensor: sensors) sensor.toJSON(sensorsJson.add<JsonObject>());
        JsonArray publishersJson = json["publishers"].to<JsonArray>();
        for (const PublisherStatus &publisher: publishers) publisher.toJSON(publishersJson.add<JsonObject>());
    }
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

    void toJSON(JsonObject json, bool includeSmoothingApplied) const {
        json["recordedAt"] = recordedAt;
        json["temperatureC"] = temperatureC;
        json["humidityPct"] = humidityPct;
        json["pressureHpa"] = pressureHpa;
        json["windSpeedMps"] = windSpeedMps;
        json["windGustMps"] = windGustMps;
        json["windDirectionDeg"] = windDirectionDeg;
        json["rainfallMm"] = rainfallMm;
        json["illuminanceLux"] = illuminanceLux;
        if (includeSmoothingApplied) json["smoothingApplied"] = true;
    }

    static void writeArray(JsonArray array, const std::vector<WeatherSample> &history) {
        for (const WeatherSample &sample: history) sample.toJSON(array.add<JsonObject>());
    }
};

class LatestSensorReadings : public HttpJsonSerializable {
public:
    WeatherSample latest;
    WeatherSample smoothed;
    bool smoothingApplied = true;

    void toJSON(JsonObject json) const override {
        latest.toJSON(json["latest"].to<JsonObject>());
        smoothed.toJSON(json["smoothed"].to<JsonObject>(), smoothingApplied);
    }
};
