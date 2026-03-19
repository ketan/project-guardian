#include "models.h"

namespace {
    void writeStringArray(JsonArray array, const std::vector<String> &values) {
        for (const String &value: values) {
            array.add(value);
        }
    }

    void parseStringArray(JsonArray array, std::vector<String> &values) {
        values.clear();
        for (JsonVariant value: array) {
            values.push_back(value.as<String>());
        }
    }

    void writeWeatherSample(JsonObject root, const WeatherSample &sample, bool includeSmoothingApplied = false) {
        root["recordedAt"] = sample.recordedAt;
        root["temperatureC"] = sample.temperatureC;
        root["humidityPct"] = sample.humidityPct;
        root["pressureHpa"] = sample.pressureHpa;
        root["windSpeedMps"] = sample.windSpeedMps;
        root["windGustMps"] = sample.windGustMps;
        root["windDirectionDeg"] = sample.windDirectionDeg;
        root["rainfallMm"] = sample.rainfallMm;
        root["illuminanceLux"] = sample.illuminanceLux;

        if (includeSmoothingApplied) {
            root["smoothingApplied"] = true;
        }
    }
}

void GeoLocation::toJSON(JsonObject json) const {
    json["latitude"] = latitude;
    json["longitude"] = longitude;
    json["elevationMeters"] = elevationMeters;
}

bool GeoLocation::fromJSON(JsonObject json) {
    if (!json["latitude"].is<double>() || !json["longitude"].is<double>()) {
        return false;
    }

    latitude = json["latitude"].as<double>();
    longitude = json["longitude"].as<double>();
    elevationMeters = json["elevationMeters"] | 0.0;
    return true;
}

void StationConfig::toJSON(JsonObject json) const {
    json["stationName"] = stationName;
    json["locationFromGPS"] = locationFromGPS;
    json["gpsPollIntervalHours"] = gpsPollIntervalHours;
    if (hasLocation) {
        location.toJSON(json["location"].to<JsonObject>());
    }
    json["notes"] = notes;
}

bool StationConfig::fromJSON(JsonObject json) {
    if (!json["stationName"].is<String>() || !json["locationFromGPS"].is<bool>() ||
        !json["gpsPollIntervalHours"].is<int>()) {
        return false;
    }

    stationName = json["stationName"].as<String>();
    locationFromGPS = json["locationFromGPS"].as<bool>();
    gpsPollIntervalHours = json["gpsPollIntervalHours"].as<int>();
    notes = json["notes"] | "";

    if (json["location"].is<JsonObject>()) {
        hasLocation = location.fromJSON(json["location"].as<JsonObject>());
    } else {
        hasLocation = false;
    }
    return hasLocation || !json["location"].is<JsonObject>();
}

void SamplingConfig::toJSON(JsonObject json) const {
    json["intervalSeconds"] = intervalSeconds;
    json["adminWindowMinutes"] = adminWindowMinutes;
    json["deepSleepEnabled"] = deepSleepEnabled;
    json["wakeDurationSeconds"] = wakeDurationSeconds;
    json["historyAggregationMinutes"] = historyAggregationMinutes;
}

bool SamplingConfig::fromJSON(JsonObject json) {
    if (!json["intervalSeconds"].is<int>() || !json["adminWindowMinutes"].is<int>() ||
        !json["deepSleepEnabled"].is<bool>()) {
        return false;
    }

    intervalSeconds = json["intervalSeconds"].as<int>();
    adminWindowMinutes = json["adminWindowMinutes"].as<int>();
    deepSleepEnabled = json["deepSleepEnabled"].as<bool>();
    wakeDurationSeconds = json["wakeDurationSeconds"] | wakeDurationSeconds;
    historyAggregationMinutes = json["historyAggregationMinutes"] | historyAggregationMinutes;
    return true;
}

void SmoothingFieldConfig::toJSON(JsonObject json) const {
    json["metric"] = metric;
    json["method"] = method;
    if (hasWindowSamples) {
        json["windowSamples"] = windowSamples;
    }
    if (hasAlpha) {
        json["alpha"] = alpha;
    }
}

bool SmoothingFieldConfig::fromJSON(JsonObject json) {
    if (!json["metric"].is<String>() || !json["method"].is<String>()) {
        return false;
    }
    metric = json["metric"].as<String>();
    method = json["method"].as<String>();
    hasWindowSamples = json["windowSamples"].is<int>();
    if (hasWindowSamples) {
        windowSamples = json["windowSamples"].as<int>();
    }
    hasAlpha = json["alpha"].is<double>();
    if (hasAlpha) {
        alpha = json["alpha"].as<double>();
    }
    return true;
}

void SmoothingConfig::toJSON(JsonObject json) const {
    json["enabled"] = enabled;
    JsonArray fieldsArray = json["fields"].to<JsonArray>();
    for (const SmoothingFieldConfig &field: fields) {
        field.toJSON(fieldsArray.add<JsonObject>());
    }
}

bool SmoothingConfig::fromJSON(JsonObject json) {
    if (!json["enabled"].is<bool>() || !json["fields"].is<JsonArray>()) {
        return false;
    }
    enabled = json["enabled"].as<bool>();
    fields.clear();
    for (JsonObject item: json["fields"].as<JsonArray>()) {
        SmoothingFieldConfig field;
        if (!field.fromJSON(item)) {
            return false;
        }
        fields.push_back(field);
    }
    return true;
}

void StorageConfig::toJSON(JsonObject json) const {
    json["retentionDays"] = retentionDays;
    json["logFormat"] = logFormat;
    json["configSource"] = configSource;
}

bool StorageConfig::fromJSON(JsonObject json) {
    if (!json["retentionDays"].is<int>() || !json["logFormat"].is<String>()) {
        return false;
    }
    retentionDays = json["retentionDays"].as<int>();
    logFormat = json["logFormat"].as<String>();
    configSource = json["configSource"] | configSource;
    return true;
}

void WifiConfig::toJSON(JsonObject json) const {
    json["enabled"] = enabled;
    json["ssid"] = ssid;
    if (!password.isEmpty()) {
        json["password"] = password;
    }
    json["passwordConfigured"] = passwordConfigured;
    writeStringArray(json["allowedOrigins"].to<JsonArray>(), allowedOrigins);
}

bool WifiConfig::fromJSON(JsonObject json) {
    enabled = json["enabled"] | enabled;
    ssid = json["ssid"] | ssid;
    password = json["password"] | password;
    passwordConfigured = json["passwordConfigured"] | passwordConfigured;
    if (json["allowedOrigins"].is<JsonArray>()) {
        parseStringArray(json["allowedOrigins"].as<JsonArray>(), allowedOrigins);
    }
    return true;
}

void CellularConfig::toJSON(JsonObject json) const {
    json["enabled"] = enabled;
    json["modemType"] = modemType;
    if (!apn.isEmpty()) {
        json["apn"] = apn;
    }
    if (!pin.isEmpty()) {
        json["pin"] = pin;
    }
    json["pinConfigured"] = pinConfigured;
    json["smsEnabled"] = smsEnabled;
}

bool CellularConfig::fromJSON(JsonObject json) {
    enabled = json["enabled"] | enabled;
    modemType = json["modemType"] | modemType;
    apn = json["apn"] | apn;
    pin = json["pin"] | pin;
    pinConfigured = json["pinConfigured"] | pinConfigured;
    smsEnabled = json["smsEnabled"] | smsEnabled;
    return true;
}

void NetworkConfig::toJSON(JsonObject json) const {
    json["preferredTransport"] = preferredTransport;
    wifi.toJSON(json["wifi"].to<JsonObject>());
    cellular.toJSON(json["cellular"].to<JsonObject>());
}

bool NetworkConfig::fromJSON(JsonObject json) {
    if (!json["preferredTransport"].is<String>() || !json["wifi"].is<JsonObject>() ||
        !json["cellular"].is<JsonObject>()) {
        return false;
    }
    preferredTransport = json["preferredTransport"].as<String>();
    return wifi.fromJSON(json["wifi"].as<JsonObject>()) &&
           cellular.fromJSON(json["cellular"].as<JsonObject>());
}

void PhoneWhitelistEntry::toJSON(JsonObject json) const {
    json["label"] = label;
    json["phoneNumber"] = phoneNumber;
}

bool PhoneWhitelistEntry::fromJSON(JsonObject json) {
    if (!json["label"].is<String>() || !json["phoneNumber"].is<String>()) {
        return false;
    }
    label = json["label"].as<String>();
    phoneNumber = json["phoneNumber"].as<String>();
    return true;
}

void SmsAdminConfig::toJSON(JsonObject json) const {
    json["enabled"] = enabled;
    JsonArray whitelistArray = json["whitelist"].to<JsonArray>();
    for (const PhoneWhitelistEntry &entry: whitelist) {
        entry.toJSON(whitelistArray.add<JsonObject>());
    }
}

bool SmsAdminConfig::fromJSON(JsonObject json) {
    if (!json["enabled"].is<bool>() || !json["whitelist"].is<JsonArray>()) {
        return false;
    }
    enabled = json["enabled"].as<bool>();
    whitelist.clear();
    for (JsonObject item: json["whitelist"].as<JsonArray>()) {
        PhoneWhitelistEntry entry;
        if (!entry.fromJSON(item)) {
            return false;
        }
        whitelist.push_back(entry);
    }
    return true;
}

void WebUiConfig::toJSON(JsonObject json) const {
    json["tokenTtlMinutes"] = tokenTtlMinutes;
    writeStringArray(json["allowedOrigins"].to<JsonArray>(), allowedOrigins);
}

bool WebUiConfig::fromJSON(JsonObject json) {
    if (!json["tokenTtlMinutes"].is<int>() || !json["allowedOrigins"].is<JsonArray>()) {
        return false;
    }
    tokenTtlMinutes = json["tokenTtlMinutes"].as<int>();
    parseStringArray(json["allowedOrigins"].as<JsonArray>(), allowedOrigins);
    return true;
}

void SensorConfig::toJSON(JsonObject json) const {
    json["id"] = id;
    json["type"] = type;
    json["enabled"] = enabled;
    if (!transport.isEmpty()) {
        json["transport"] = transport;
    }
    if (hasPollIntervalSeconds) {
        json["pollIntervalSeconds"] = pollIntervalSeconds;
    }
    if (hasAddress) {
        json["address"] = address;
    }
}

bool SensorConfig::fromJSON(JsonObject json) {
    if (!json["id"].is<String>() || !json["type"].is<String>() || !json["enabled"].is<bool>()) {
        return false;
    }
    id = json["id"].as<String>();
    type = json["type"].as<String>();
    enabled = json["enabled"].as<bool>();
    transport = json["transport"] | "";
    hasPollIntervalSeconds = json["pollIntervalSeconds"].is<int>();
    if (hasPollIntervalSeconds) {
        pollIntervalSeconds = json["pollIntervalSeconds"].as<int>();
    }
    hasAddress = json["address"].is<int>();
    if (hasAddress) {
        address = json["address"].as<int>();
    }
    return true;
}

void SensorConfig::writeArray(JsonArray array, const std::vector<SensorConfig> &sensors) {
    for (const SensorConfig &sensor: sensors) {
        sensor.toJSON(array.add<JsonObject>());
    }
}

bool SensorConfig::parseArray(JsonArray array, std::vector<SensorConfig> &sensors) {
    sensors.clear();
    for (JsonObject item: array) {
        SensorConfig sensor;
        if (!sensor.fromJSON(item)) {
            return false;
        }
        sensors.push_back(sensor);
    }
    return true;
}

void WundergroundPublisherConfig::toJSON(JsonObject json) const {
    json["type"] = type;
    json["enabled"] = enabled;
    json["publishIntervalSeconds"] = publishIntervalSeconds;
    if (hasIncludeHistoryWindowMinutes) {
        json["includeHistoryWindowMinutes"] = includeHistoryWindowMinutes;
    }
    json["stationId"] = stationId;
    json["apiKeyConfigured"] = apiKeyConfigured;
}

bool WundergroundPublisherConfig::fromJSON(JsonObject json) {
    if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
        !json["publishIntervalSeconds"].is<int>() || !json["stationId"].is<String>()) {
        return false;
    }
    type = json["type"].as<String>();
    enabled = json["enabled"].as<bool>();
    publishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
    stationId = json["stationId"].as<String>();
    apiKey = json["apiKey"] | "";
    apiKeyConfigured = !apiKey.isEmpty() || apiKeyConfigured;
    hasIncludeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].is<int>();
    if (hasIncludeHistoryWindowMinutes) {
        includeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].as<int>();
    }
    return true;
}

void WindyPublisherConfig::toJSON(JsonObject json) const {
    json["type"] = type;
    json["enabled"] = enabled;
    json["publishIntervalSeconds"] = publishIntervalSeconds;
    if (hasIncludeHistoryWindowMinutes) {
        json["includeHistoryWindowMinutes"] = includeHistoryWindowMinutes;
    }
    json["stationId"] = stationId;
    json["apiKeyConfigured"] = apiKeyConfigured;
}

bool WindyPublisherConfig::fromJSON(JsonObject json) {
    if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
        !json["publishIntervalSeconds"].is<int>() || !json["stationId"].is<String>()) {
        return false;
    }
    type = json["type"].as<String>();
    enabled = json["enabled"].as<bool>();
    publishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
    stationId = json["stationId"].as<String>();
    apiKey = json["apiKey"] | "";
    apiKeyConfigured = !apiKey.isEmpty() || apiKeyConfigured;
    hasIncludeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].is<int>();
    if (hasIncludeHistoryWindowMinutes) {
        includeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].as<int>();
    }
    return true;
}

void MqttPublisherConfig::toJSON(JsonObject json) const {
    json["type"] = type;
    json["enabled"] = enabled;
    json["publishIntervalSeconds"] = publishIntervalSeconds;
    if (hasIncludeHistoryWindowMinutes) {
        json["includeHistoryWindowMinutes"] = includeHistoryWindowMinutes;
    }
    json["brokerUrl"] = brokerUrl;
    json["topic"] = topic;
    json["username"] = username;
    json["passwordConfigured"] = passwordConfigured;
}

bool MqttPublisherConfig::fromJSON(JsonObject json) {
    if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
        !json["publishIntervalSeconds"].is<int>() || !json["brokerUrl"].is<String>() ||
        !json["topic"].is<String>() || !json["username"].is<String>()) {
        return false;
    }
    type = json["type"].as<String>();
    enabled = json["enabled"].as<bool>();
    publishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
    brokerUrl = json["brokerUrl"].as<String>();
    topic = json["topic"].as<String>();
    username = json["username"].as<String>();
    password = json["password"] | "";
    passwordConfigured = !password.isEmpty() || passwordConfigured;
    hasIncludeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].is<int>();
    if (hasIncludeHistoryWindowMinutes) {
        includeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].as<int>();
    }
    return true;
}

void PublishersConfig::toJSON(JsonObject json) const {
    wunderground.toJSON(json["wunderground"].to<JsonObject>());
    windy.toJSON(json["windy"].to<JsonObject>());
    mqtt.toJSON(json["mqtt"].to<JsonObject>());
}

bool PublishersConfig::fromJSON(JsonObject json) {
    if (!json["wunderground"].is<JsonObject>() || !json["windy"].is<JsonObject>() || !json["mqtt"].is<JsonObject>()) {
        return false;
    }
    return wunderground.fromJSON(json["wunderground"].as<JsonObject>()) &&
           windy.fromJSON(json["windy"].as<JsonObject>()) &&
           mqtt.fromJSON(json["mqtt"].as<JsonObject>());
}

void DeviceConfig::toJSON(JsonObject json) const {
    json["schemaVersion"] = schemaVersion;
    station.toJSON(json["station"].to<JsonObject>());
    sampling.toJSON(json["sampling"].to<JsonObject>());
    smoothing.toJSON(json["smoothing"].to<JsonObject>());
    storage.toJSON(json["storage"].to<JsonObject>());
    network.toJSON(json["network"].to<JsonObject>());
    smsAdmin.toJSON(json["smsAdmin"].to<JsonObject>());
    webUi.toJSON(json["webUi"].to<JsonObject>());
    SensorConfig::writeArray(json["sensors"].to<JsonArray>(), sensors);
    publishers.toJSON(json["publishers"].to<JsonObject>());
}

bool DeviceConfig::fromJSON(JsonObject json) {
    if (!json["station"].is<JsonObject>() || !json["sampling"].is<JsonObject>() || !json["smoothing"].is<JsonObject>() ||
        !json["storage"].is<JsonObject>() || !json["network"].is<JsonObject>() || !json["smsAdmin"].is<JsonObject>() ||
        !json["webUi"].is<JsonObject>() || !json["sensors"].is<JsonArray>() || !json["publishers"].is<JsonObject>()) {
        return false;
    }

    schemaVersion = json["schemaVersion"] | schemaVersion;

    return station.fromJSON(json["station"].as<JsonObject>()) &&
           sampling.fromJSON(json["sampling"].as<JsonObject>()) &&
           smoothing.fromJSON(json["smoothing"].as<JsonObject>()) &&
           storage.fromJSON(json["storage"].as<JsonObject>()) &&
           network.fromJSON(json["network"].as<JsonObject>()) &&
           smsAdmin.fromJSON(json["smsAdmin"].as<JsonObject>()) &&
           webUi.fromJSON(json["webUi"].as<JsonObject>()) &&
           SensorConfig::parseArray(json["sensors"].as<JsonArray>(), sensors) &&
           publishers.fromJSON(json["publishers"].as<JsonObject>());
}

namespace ModelJson {
    void writeDeviceStatus(JsonObject root, const DeviceStatus &status) {
        JsonObject device = root["device"].to<JsonObject>();
        device["deviceId"] = status.device.deviceId;
        device["firmwareVersion"] = status.device.firmwareVersion;
        device["hardwareModel"] = status.device.hardwareModel;
        device["uptimeSeconds"] = status.device.uptimeSeconds;
        device["currentTime"] = status.device.currentTime;
        device["lastBootReason"] = status.device.lastBootReason;

        JsonObject connectivity = root["connectivity"].to<JsonObject>();
        JsonObject wifi = connectivity["wifi"].to<JsonObject>();
        wifi["enabled"] = status.connectivity.wifi.enabled;
        wifi["active"] = status.connectivity.wifi.active;
        wifi["connected"] = status.connectivity.wifi.connected;
        wifi["ssid"] = status.connectivity.wifi.ssid;
        wifi["ipAddress"] = status.connectivity.wifi.ipAddress;
        wifi["rssiDbm"] = status.connectivity.wifi.rssiDbm;

        JsonObject cellular = connectivity["cellular"].to<JsonObject>();
        cellular["enabled"] = status.connectivity.cellular.enabled;
        cellular["active"] = status.connectivity.cellular.active;
        cellular["registered"] = status.connectivity.cellular.registered;
        cellular["modemType"] = status.connectivity.cellular.modemType;
        cellular["operatorName"] = status.connectivity.cellular.operatorName;
        cellular["signalQuality"] = status.connectivity.cellular.signalQuality;
        cellular["ipv4"] = status.connectivity.cellular.ipv4;
        cellular["ipv6"] = status.connectivity.cellular.ipv6;

        JsonObject storage = root["storage"].to<JsonObject>();
        storage["sdCardPresent"] = status.storage.sdCardPresent;
        storage["freeBytes"] = status.storage.freeBytes;
        storage["usedBytes"] = status.storage.usedBytes;
        storage["retentionDays"] = status.storage.retentionDays;
        storage["oldestRecordAt"] = status.storage.oldestRecordAt;
        storage["newestRecordAt"] = status.storage.newestRecordAt;

        JsonObject sampling = root["sampling"].to<JsonObject>();
        sampling["intervalSeconds"] = status.sampling.intervalSeconds;
        sampling["nextSampleAt"] = status.sampling.nextSampleAt;
        sampling["lastSampleAt"] = status.sampling.lastSampleAt;
        sampling["sleepEnabled"] = status.sampling.sleepEnabled;
        sampling["smoothingEnabled"] = status.sampling.smoothingEnabled;

        JsonObject adminWindow = root["adminWindow"].to<JsonObject>();
        adminWindow["active"] = status.adminWindow.active;
        if (!status.adminWindow.openedAt.isEmpty()) {
            adminWindow["openedAt"] = status.adminWindow.openedAt;
        }
        if (!status.adminWindow.expiresAt.isEmpty()) {
            adminWindow["expiresAt"] = status.adminWindow.expiresAt;
        }
        if (!status.adminWindow.requestedBy.isEmpty()) {
            adminWindow["requestedBy"] = status.adminWindow.requestedBy;
        }

        JsonArray sensors = root["sensors"].to<JsonArray>();
        for (const SensorStatus &sensorStatus: status.sensors) {
            JsonObject sensor = sensors.add<JsonObject>();
            sensor["id"] = sensorStatus.id;
            sensor["kind"] = sensorStatus.kind;
            sensor["enabled"] = sensorStatus.enabled;
            sensor["healthy"] = sensorStatus.healthy;
            sensor["lastReadAt"] = sensorStatus.lastReadAt;
            sensor["message"] = sensorStatus.message;
        }

        JsonArray publishers = root["publishers"].to<JsonArray>();
        for (const PublisherStatus &publisherStatus: status.publishers) {
            JsonObject publisher = publishers.add<JsonObject>();
            publisher["type"] = publisherStatus.type;
            publisher["enabled"] = publisherStatus.enabled;
            if (!publisherStatus.lastPublishAt.isEmpty()) {
                publisher["lastPublishAt"] = publisherStatus.lastPublishAt;
            }
            publisher["lastResult"] = publisherStatus.lastResult;
            publisher["message"] = publisherStatus.message;
        }
    }

    void writeLatestSensorReadings(JsonObject root, const LatestSensorReadings &readings) {
        writeWeatherSample(root["latest"].to<JsonObject>(), readings.latest);
        writeWeatherSample(root["smoothed"].to<JsonObject>(), readings.smoothed, readings.smoothingApplied);
    }

    void writeHistory(JsonObject root, const std::vector<WeatherSample> &history) {
        JsonArray samples = root["samples"].to<JsonArray>();
        for (const WeatherSample &sample: history) {
            writeWeatherSample(samples.add<JsonObject>(), sample);
        }
    }

    void writeOtaUploadResult(JsonObject root, const OtaUploadResult &result) {
        root["checksumVerified"] = result.checksumVerified;
        root["staged"] = result.staged;
        root["rebootScheduled"] = result.rebootScheduled;
        root["firmwareSizeBytes"] = result.firmwareSizeBytes;
        if (!result.stagedPath.isEmpty()) {
            root["stagedPath"] = result.stagedPath;
        }
        root["message"] = result.message;
    }
}
