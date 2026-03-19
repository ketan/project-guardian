#include "json_codec.h"

namespace {
    void writeGeoLocation(JsonObject root, const GeoLocation &location) {
        root["latitude"] = location.latitude;
        root["longitude"] = location.longitude;
        root["elevationMeters"] = location.elevationMeters;
    }

    bool parseGeoLocation(JsonObject json, GeoLocation &location) {
        if (!json["latitude"].is<double>() || !json["longitude"].is<double>()) {
            return false;
        }

        location.latitude = json["latitude"].as<double>();
        location.longitude = json["longitude"].as<double>();
        location.elevationMeters = json["elevationMeters"] | 0.0;
        return true;
    }

    void writeStringArray(JsonArray root, const std::vector<String> &values) {
        for (const String &value: values) {
            root.add(value);
        }
    }

    void parseStringArray(JsonArray json, std::vector<String> &values) {
        values.clear();
        for (JsonVariant value: json) {
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

    template<typename T>
    void logUpdatedSection(const char *label, const T &writer) {
        JsonDocument doc;
        writer(doc.to<JsonVariant>());
        serializeJsonPretty(doc, Serial);
        Serial.println();
    }
} // namespace

void writeDeviceStatus(JsonObject root, const AppState &state) {
    JsonObject device = root["device"].to<JsonObject>();
    device["deviceId"] = state.status.device.deviceId;
    device["firmwareVersion"] = state.status.device.firmwareVersion;
    device["hardwareModel"] = state.status.device.hardwareModel;
    device["uptimeSeconds"] = state.status.device.uptimeSeconds;
    device["currentTime"] = state.status.device.currentTime;
    device["lastBootReason"] = state.status.device.lastBootReason;

    JsonObject connectivity = root["connectivity"].to<JsonObject>();
    JsonObject wifi = connectivity["wifi"].to<JsonObject>();
    wifi["enabled"] = state.status.connectivity.wifi.enabled;
    wifi["active"] = state.status.connectivity.wifi.active;
    wifi["connected"] = state.status.connectivity.wifi.connected;
    wifi["ssid"] = state.status.connectivity.wifi.ssid;
    wifi["ipAddress"] = state.status.connectivity.wifi.ipAddress;
    wifi["rssiDbm"] = state.status.connectivity.wifi.rssiDbm;

    JsonObject cellular = connectivity["cellular"].to<JsonObject>();
    cellular["enabled"] = state.status.connectivity.cellular.enabled;
    cellular["active"] = state.status.connectivity.cellular.active;
    cellular["registered"] = state.status.connectivity.cellular.registered;
    cellular["modemType"] = state.status.connectivity.cellular.modemType;
    cellular["operatorName"] = state.status.connectivity.cellular.operatorName;
    cellular["signalQuality"] = state.status.connectivity.cellular.signalQuality;
    cellular["ipv4"] = state.status.connectivity.cellular.ipv4;
    cellular["ipv6"] = state.status.connectivity.cellular.ipv6;

    JsonObject storage = root["storage"].to<JsonObject>();
    storage["sdCardPresent"] = state.status.storage.sdCardPresent;
    storage["freeBytes"] = state.status.storage.freeBytes;
    storage["usedBytes"] = state.status.storage.usedBytes;
    storage["retentionDays"] = state.status.storage.retentionDays;
    storage["oldestRecordAt"] = state.status.storage.oldestRecordAt;
    storage["newestRecordAt"] = state.status.storage.newestRecordAt;

    JsonObject sampling = root["sampling"].to<JsonObject>();
    sampling["intervalSeconds"] = state.status.sampling.intervalSeconds;
    sampling["nextSampleAt"] = state.status.sampling.nextSampleAt;
    sampling["lastSampleAt"] = state.status.sampling.lastSampleAt;
    sampling["sleepEnabled"] = state.status.sampling.sleepEnabled;
    sampling["smoothingEnabled"] = state.status.sampling.smoothingEnabled;

    JsonObject adminWindow = root["adminWindow"].to<JsonObject>();
    adminWindow["active"] = state.status.adminWindow.active;
    if (!state.status.adminWindow.openedAt.isEmpty()) {
        adminWindow["openedAt"] = state.status.adminWindow.openedAt;
    }
    if (!state.status.adminWindow.expiresAt.isEmpty()) {
        adminWindow["expiresAt"] = state.status.adminWindow.expiresAt;
    }
    if (!state.status.adminWindow.requestedBy.isEmpty()) {
        adminWindow["requestedBy"] = state.status.adminWindow.requestedBy;
    }

    JsonArray sensors = root["sensors"].to<JsonArray>();
    for (const SensorStatus &sensorStatus: state.status.sensors) {
        JsonObject sensor = sensors.add<JsonObject>();
        sensor["id"] = sensorStatus.id;
        sensor["kind"] = sensorStatus.kind;
        sensor["enabled"] = sensorStatus.enabled;
        sensor["healthy"] = sensorStatus.healthy;
        sensor["lastReadAt"] = sensorStatus.lastReadAt;
        sensor["message"] = sensorStatus.message;
    }

    JsonArray publishers = root["publishers"].to<JsonArray>();
    for (const PublisherStatus &publisherStatus: state.status.publishers) {
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

void writeStationConfig(JsonObject root, const StationConfig &config) {
    root["stationName"] = config.stationName;
    root["locationFromGPS"] = config.locationFromGPS;
    root["gpsPollIntervalHours"] = config.gpsPollIntervalHours;
    if (config.hasLocation) {
        JsonObject location = root["location"].to<JsonObject>();
        writeGeoLocation(location, config.location);
    }
    root["notes"] = config.notes;
}

void writeSamplingConfig(JsonObject root, const SamplingConfig &config) {
    root["intervalSeconds"] = config.intervalSeconds;
    root["adminWindowMinutes"] = config.adminWindowMinutes;
    root["deepSleepEnabled"] = config.deepSleepEnabled;
    root["wakeDurationSeconds"] = config.wakeDurationSeconds;
    root["historyAggregationMinutes"] = config.historyAggregationMinutes;
}

void writeSmoothingConfig(JsonObject root, const SmoothingConfig &config) {
    root["enabled"] = config.enabled;
    JsonArray fields = root["fields"].to<JsonArray>();
    for (const SmoothingFieldConfig &field: config.fields) {
        JsonObject item = fields.add<JsonObject>();
        item["metric"] = field.metric;
        item["method"] = field.method;
        if (field.hasWindowSamples) {
            item["windowSamples"] = field.windowSamples;
        }
        if (field.hasAlpha) {
            item["alpha"] = field.alpha;
        }
    }
}

void writeStorageConfig(JsonObject root, const StorageConfig &config) {
    root["retentionDays"] = config.retentionDays;
    root["logFormat"] = config.logFormat;
    root["configSource"] = config.configSource;
}

void writeNetworkConfig(JsonObject root, const NetworkConfig &config) {
    root["preferredTransport"] = config.preferredTransport;

    JsonObject wifi = root["wifi"].to<JsonObject>();
    wifi["enabled"] = config.wifi.enabled;
    wifi["ssid"] = config.wifi.ssid;
    if (!config.wifi.password.isEmpty()) {
        wifi["password"] = config.wifi.password;
    }
    wifi["passwordConfigured"] = config.wifi.passwordConfigured;
    writeStringArray(wifi["allowedOrigins"].to<JsonArray>(), config.wifi.allowedOrigins);

    JsonObject cellular = root["cellular"].to<JsonObject>();
    cellular["enabled"] = config.cellular.enabled;
    cellular["modemType"] = config.cellular.modemType;
    if (!config.cellular.apn.isEmpty()) {
        cellular["apn"] = config.cellular.apn;
    }
    if (!config.cellular.pin.isEmpty()) {
        cellular["pin"] = config.cellular.pin;
    }
    cellular["pinConfigured"] = config.cellular.pinConfigured;
    cellular["smsEnabled"] = config.cellular.smsEnabled;
}

void writeSmsAdminConfig(JsonObject root, const SmsAdminConfig &config) {
    root["enabled"] = config.enabled;
    JsonArray whitelist = root["whitelist"].to<JsonArray>();
    for (const PhoneWhitelistEntry &entry: config.whitelist) {
        JsonObject item = whitelist.add<JsonObject>();
        item["label"] = entry.label;
        item["phoneNumber"] = entry.phoneNumber;
    }
}

void writeWebUiConfig(JsonObject root, const WebUiConfig &config) {
    root["tokenTtlMinutes"] = config.tokenTtlMinutes;
    writeStringArray(root["allowedOrigins"].to<JsonArray>(), config.allowedOrigins);
}

void writeSensorsConfig(JsonArray root, const std::vector<SensorConfig> &sensors) {
    for (const SensorConfig &sensorConfig: sensors) {
        JsonObject sensor = root.add<JsonObject>();
        sensor["id"] = sensorConfig.id;
        sensor["type"] = sensorConfig.type;
        sensor["enabled"] = sensorConfig.enabled;
        if (!sensorConfig.transport.isEmpty()) {
            sensor["transport"] = sensorConfig.transport;
        }
        if (sensorConfig.hasPollIntervalSeconds) {
            sensor["pollIntervalSeconds"] = sensorConfig.pollIntervalSeconds;
        }
        if (sensorConfig.hasAddress) {
            sensor["address"] = sensorConfig.address;
        }
    }
}

void writeWundergroundPublisher(JsonObject root, const WundergroundPublisherConfig &config) {
    root["type"] = config.type;
    root["enabled"] = config.enabled;
    root["publishIntervalSeconds"] = config.publishIntervalSeconds;
    if (config.hasIncludeHistoryWindowMinutes) {
        root["includeHistoryWindowMinutes"] = config.includeHistoryWindowMinutes;
    }
    root["stationId"] = config.stationId;
    root["apiKeyConfigured"] = config.apiKeyConfigured;
}

void writeWindyPublisher(JsonObject root, const WindyPublisherConfig &config) {
    root["type"] = config.type;
    root["enabled"] = config.enabled;
    root["publishIntervalSeconds"] = config.publishIntervalSeconds;
    if (config.hasIncludeHistoryWindowMinutes) {
        root["includeHistoryWindowMinutes"] = config.includeHistoryWindowMinutes;
    }
    root["stationId"] = config.stationId;
    root["apiKeyConfigured"] = config.apiKeyConfigured;
}

void writeMqttPublisher(JsonObject root, const MqttPublisherConfig &config) {
    root["type"] = config.type;
    root["enabled"] = config.enabled;
    root["publishIntervalSeconds"] = config.publishIntervalSeconds;
    if (config.hasIncludeHistoryWindowMinutes) {
        root["includeHistoryWindowMinutes"] = config.includeHistoryWindowMinutes;
    }
    root["brokerUrl"] = config.brokerUrl;
    root["topic"] = config.topic;
    root["username"] = config.username;
    root["passwordConfigured"] = config.passwordConfigured;
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

bool parseStationConfig(JsonObject json, StationConfig &config) {
    if (!json["stationName"].is<String>() || !json["locationFromGPS"].is<bool>() ||
        !json["gpsPollIntervalHours"].is<int>()) {
        return false;
    }
    config.stationName = json["stationName"].as<String>();
    config.locationFromGPS = json["locationFromGPS"].as<bool>();
    config.gpsPollIntervalHours = json["gpsPollIntervalHours"].as<int>();
    config.notes = json["notes"] | "";
    if (json["location"].is<JsonObject>()) {
        config.hasLocation = parseGeoLocation(json["location"].as<JsonObject>(), config.location);
    } else {
        config.hasLocation = false;
    }
    return true;
}

bool parseSamplingConfig(JsonObject json, SamplingConfig &config) {
    if (!json["intervalSeconds"].is<int>() || !json["adminWindowMinutes"].is<int>() ||
        !json["deepSleepEnabled"].is<bool>()) {
        return false;
    }
    config.intervalSeconds = json["intervalSeconds"].as<int>();
    config.adminWindowMinutes = json["adminWindowMinutes"].as<int>();
    config.deepSleepEnabled = json["deepSleepEnabled"].as<bool>();
    config.wakeDurationSeconds = json["wakeDurationSeconds"] | config.wakeDurationSeconds;
    config.historyAggregationMinutes = json["historyAggregationMinutes"] | config.historyAggregationMinutes;
    return true;
}

bool parseSmoothingConfig(JsonObject json, SmoothingConfig &config) {
    if (!json["enabled"].is<bool>() || !json["fields"].is<JsonArray>()) {
        return false;
    }
    config.enabled = json["enabled"].as<bool>();
    config.fields.clear();
    for (JsonObject item: json["fields"].as<JsonArray>()) {
        if (!item["metric"].is<String>() || !item["method"].is<String>()) {
            return false;
        }
        SmoothingFieldConfig field;
        field.metric = item["metric"].as<String>();
        field.method = item["method"].as<String>();
        if (item["windowSamples"].is<int>()) {
            field.hasWindowSamples = true;
            field.windowSamples = item["windowSamples"].as<int>();
        }
        if (item["alpha"].is<double>()) {
            field.hasAlpha = true;
            field.alpha = item["alpha"].as<double>();
        }
        config.fields.push_back(field);
    }
    return true;
}

bool parseStorageConfig(JsonObject json, StorageConfig &config) {
    if (!json["retentionDays"].is<int>() || !json["logFormat"].is<String>()) {
        return false;
    }
    config.retentionDays = json["retentionDays"].as<int>();
    config.logFormat = json["logFormat"].as<String>();
    config.configSource = json["configSource"] | config.configSource;
    return true;
}

bool parseNetworkConfig(JsonObject json, NetworkConfig &config) {
    if (!json["preferredTransport"].is<String>() || !json["wifi"].is<JsonObject>() ||
        !json["cellular"].is<JsonObject>()) {
        return false;
    }
    config.preferredTransport = json["preferredTransport"].as<String>();

    JsonObject wifi = json["wifi"].as<JsonObject>();
    config.wifi.enabled = wifi["enabled"] | config.wifi.enabled;
    config.wifi.ssid = wifi["ssid"] | config.wifi.ssid;
    config.wifi.password = wifi["password"] | config.wifi.password;
    config.wifi.passwordConfigured = wifi["passwordConfigured"] | config.wifi.passwordConfigured;
    if (wifi["allowedOrigins"].is<JsonArray>()) {
        parseStringArray(wifi["allowedOrigins"].as<JsonArray>(), config.wifi.allowedOrigins);
    }

    JsonObject cellular = json["cellular"].as<JsonObject>();
    config.cellular.enabled = cellular["enabled"] | config.cellular.enabled;
    config.cellular.modemType = cellular["modemType"] | config.cellular.modemType;
    config.cellular.apn = cellular["apn"] | config.cellular.apn;
    config.cellular.pin = cellular["pin"] | config.cellular.pin;
    config.cellular.pinConfigured = cellular["pinConfigured"] | config.cellular.pinConfigured;
    config.cellular.smsEnabled = cellular["smsEnabled"] | config.cellular.smsEnabled;
    return true;
}

bool parseSmsAdminConfig(JsonObject json, SmsAdminConfig &config) {
    if (!json["enabled"].is<bool>() || !json["whitelist"].is<JsonArray>()) {
        return false;
    }
    config.enabled = json["enabled"].as<bool>();
    config.whitelist.clear();
    for (JsonObject item: json["whitelist"].as<JsonArray>()) {
        if (!item["label"].is<String>() || !item["phoneNumber"].is<String>()) {
            return false;
        }
        PhoneWhitelistEntry entry;
        entry.label = item["label"].as<String>();
        entry.phoneNumber = item["phoneNumber"].as<String>();
        config.whitelist.push_back(entry);
    }
    return true;
}

bool parseWebUiConfig(JsonObject json, WebUiConfig &config) {
    if (!json["tokenTtlMinutes"].is<int>() || !json["allowedOrigins"].is<JsonArray>()) {
        return false;
    }
    config.tokenTtlMinutes = json["tokenTtlMinutes"].as<int>();
    parseStringArray(json["allowedOrigins"].as<JsonArray>(), config.allowedOrigins);
    return true;
}

bool parseSensorsConfig(JsonArray json, std::vector<SensorConfig> &sensors) {
    sensors.clear();
    for (JsonObject item: json) {
        if (!item["id"].is<String>() || !item["type"].is<String>() || !item["enabled"].is<bool>()) {
            return false;
        }
        SensorConfig sensor;
        sensor.id = item["id"].as<String>();
        sensor.type = item["type"].as<String>();
        sensor.enabled = item["enabled"].as<bool>();
        sensor.transport = item["transport"] | "";
        if (item["pollIntervalSeconds"].is<int>()) {
            sensor.hasPollIntervalSeconds = true;
            sensor.pollIntervalSeconds = item["pollIntervalSeconds"].as<int>();
        }
        if (item["address"].is<int>()) {
            sensor.hasAddress = true;
            sensor.address = item["address"].as<int>();
        }
        sensors.push_back(sensor);
    }
    return true;
}

bool parseWundergroundPublisher(JsonObject json, WundergroundPublisherConfig &config) {
    if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
        !json["publishIntervalSeconds"].is<int>() || !json["stationId"].is<String>()) {
        return false;
    }
    config.type = json["type"].as<String>();
    config.enabled = json["enabled"].as<bool>();
    config.publishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
    config.stationId = json["stationId"].as<String>();
    config.apiKey = json["apiKey"] | "";
    config.apiKeyConfigured = !config.apiKey.isEmpty() || config.apiKeyConfigured;
    if (json["includeHistoryWindowMinutes"].is<int>()) {
        config.hasIncludeHistoryWindowMinutes = true;
        config.includeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].as<int>();
    } else {
        config.hasIncludeHistoryWindowMinutes = false;
    }
    return true;
}

bool parseWindyPublisher(JsonObject json, WindyPublisherConfig &config) {
    if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
        !json["publishIntervalSeconds"].is<int>() || !json["stationId"].is<String>()) {
        return false;
    }
    config.type = json["type"].as<String>();
    config.enabled = json["enabled"].as<bool>();
    config.publishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
    config.stationId = json["stationId"].as<String>();
    config.apiKey = json["apiKey"] | "";
    config.apiKeyConfigured = !config.apiKey.isEmpty() || config.apiKeyConfigured;
    if (json["includeHistoryWindowMinutes"].is<int>()) {
        config.hasIncludeHistoryWindowMinutes = true;
        config.includeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].as<int>();
    } else {
        config.hasIncludeHistoryWindowMinutes = false;
    }
    return true;
}

bool parseMqttPublisher(JsonObject json, MqttPublisherConfig &config) {
    if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
        !json["publishIntervalSeconds"].is<int>() || !json["brokerUrl"].is<String>() ||
        !json["topic"].is<String>() || !json["username"].is<String>()) {
        return false;
    }
    config.type = json["type"].as<String>();
    config.enabled = json["enabled"].as<bool>();
    config.publishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
    config.brokerUrl = json["brokerUrl"].as<String>();
    config.topic = json["topic"].as<String>();
    config.username = json["username"].as<String>();
    config.password = json["password"] | "";
    config.passwordConfigured = !config.password.isEmpty() || config.passwordConfigured;
    if (json["includeHistoryWindowMinutes"].is<int>()) {
        config.hasIncludeHistoryWindowMinutes = true;
        config.includeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].as<int>();
    } else {
        config.hasIncludeHistoryWindowMinutes = false;
    }
    return true;
}