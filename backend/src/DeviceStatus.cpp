#include "DeviceStatus.h"

void DeviceSummary::toJSON(JsonObject json) const {
    json["deviceId"] = deviceId;
    json["firmwareVersion"] = firmwareVersion;
    json["hardwareModel"] = hardwareModel;
    json["uptimeSeconds"] = uptimeSeconds;
    json["currentTime"] = currentTime;
    json["lastBootReason"] = lastBootReason;
}

void WifiStatus::toJSON(JsonObject json) const {
    json["enabled"] = enabled;
    json["active"] = active;
    json["connected"] = connected;
    json["ssid"] = ssid;
    json["ipAddress"] = ipAddress;
    json["rssiDbm"] = rssiDbm;
}

void CellularStatus::toJSON(JsonObject json) const {
    json["enabled"] = enabled;
    json["active"] = active;
    json["registered"] = registered;
    json["modemType"] = modemType;
    json["operatorName"] = operatorName;
    json["signalQuality"] = signalQuality;
    json["ipv4"] = ipv4;
    json["ipv6"] = ipv6;
}

void ConnectivityStatus::toJSON(JsonObject json) const {
    wifi.toJSON(json["wifi"].to<JsonObject>());
    cellular.toJSON(json["cellular"].to<JsonObject>());
}

void StorageStatus::toJSON(JsonObject json) const {
    json["sdCardPresent"] = sdCardPresent;
    json["freeBytes"] = freeBytes;
    json["usedBytes"] = usedBytes;
    json["retentionDays"] = retentionDays;
    json["oldestRecordAt"] = oldestRecordAt;
    json["newestRecordAt"] = newestRecordAt;
}

void SamplingStatus::toJSON(JsonObject json) const {
    json["intervalSeconds"] = intervalSeconds;
    json["nextSampleAt"] = nextSampleAt;
    json["lastSampleAt"] = lastSampleAt;
    json["sleepEnabled"] = sleepEnabled;
    json["smoothingEnabled"] = smoothingEnabled;
}

void AdminWindowStatus::toJSON(JsonObject json) const {
    json["active"] = active;
    if (!openedAt.isEmpty()) {
        json["openedAt"] = openedAt;
    }
    if (!expiresAt.isEmpty()) {
        json["expiresAt"] = expiresAt;
    }
    if (!requestedBy.isEmpty()) {
        json["requestedBy"] = requestedBy;
    }
}

void SensorStatus::toJSON(JsonObject json) const {
    json["id"] = id;
    json["kind"] = kind;
    json["enabled"] = enabled;
    json["healthy"] = healthy;
    json["lastReadAt"] = lastReadAt;
    json["message"] = message;
}

void PublisherStatus::toJSON(JsonObject json) const {
    json["type"] = type;
    json["enabled"] = enabled;
    if (!lastPublishAt.isEmpty()) {
        json["lastPublishAt"] = lastPublishAt;
    }
    json["lastResult"] = lastResult;
    json["message"] = message;
}

void DeviceStatus::toJSON(JsonObject json) const {
    device.toJSON(json["device"].to<JsonObject>());
    connectivity.toJSON(json["connectivity"].to<JsonObject>());
    storage.toJSON(json["storage"].to<JsonObject>());
    sampling.toJSON(json["sampling"].to<JsonObject>());
    adminWindow.toJSON(json["adminWindow"].to<JsonObject>());

    JsonArray sensorsJson = json["sensors"].to<JsonArray>();
    for (const SensorStatus &sensor: sensors) {
        sensor.toJSON(sensorsJson.add<JsonObject>());
    }

    JsonArray publishersJson = json["publishers"].to<JsonArray>();
    for (const PublisherStatus &publisher: publishers) {
        publisher.toJSON(publishersJson.add<JsonObject>());
    }
}

void WeatherSample::toJSON(JsonObject json, bool includeSmoothingApplied) const {
    json["recordedAt"] = recordedAt;
    json["temperatureC"] = temperatureC;
    json["humidityPct"] = humidityPct;
    json["pressureHpa"] = pressureHpa;
    json["windSpeedMps"] = windSpeedMps;
    json["windGustMps"] = windGustMps;
    json["windDirectionDeg"] = windDirectionDeg;
    json["rainfallMm"] = rainfallMm;
    json["illuminanceLux"] = illuminanceLux;

    if (includeSmoothingApplied) {
        json["smoothingApplied"] = true;
    }
}

void WeatherSample::writeArray(JsonArray array, const std::vector<WeatherSample> &history) {
    for (const WeatherSample &sample: history) {
        sample.toJSON(array.add<JsonObject>());
    }
}

void LatestSensorReadings::toJSON(JsonObject json) const {
    latest.toJSON(json["latest"].to<JsonObject>());
    smoothed.toJSON(json["smoothed"].to<JsonObject>(), smoothingApplied);
}

