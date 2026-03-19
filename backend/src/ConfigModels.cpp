#include "ConfigModels.h"

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
    json["password"] = password;
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

void WifiConfig::toHttpResponseJSON(JsonObject json) const {
    json["enabled"] = enabled;
    json["ssid"] = ssid;
    json["passwordConfigured"] = passwordConfigured;
    writeStringArray(json["allowedOrigins"].to<JsonArray>(), allowedOrigins);
}

bool WifiConfig::fromHttpRequestJSON(JsonObject json) {
    enabled = json["enabled"] | enabled;
    ssid = json["ssid"] | ssid;

    if (json["password"].is<String>()) {
        password = json["password"].as<String>();
        passwordConfigured = !password.isEmpty();
    }

    if (json["allowedOrigins"].is<JsonArray>()) {
        parseStringArray(json["allowedOrigins"].as<JsonArray>(), allowedOrigins);
    }
    return true;
}

void CellularConfig::toJSON(JsonObject json) const {
    json["enabled"] = enabled;
    json["modemType"] = modemType;
    json["apn"] = apn;
    json["pin"] = pin;
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

void CellularConfig::toHttpResponseJSON(JsonObject json) const {
    json["enabled"] = enabled;
    json["modemType"] = modemType;
    json["apn"] = apn;
    json["pinConfigured"] = pinConfigured;
    json["smsEnabled"] = smsEnabled;
}

bool CellularConfig::fromHttpRequestJSON(JsonObject json) {
    enabled = json["enabled"] | enabled;
    modemType = json["modemType"] | modemType;
    apn = json["apn"] | apn;

    if (json["pin"].is<String>()) {
        pin = json["pin"].as<String>();
        pinConfigured = !pin.isEmpty();
    }

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

void NetworkConfig::toHttpResponseJSON(JsonObject json) const {
    json["preferredTransport"] = preferredTransport;
    wifi.toHttpResponseJSON(json["wifi"].to<JsonObject>());
    cellular.toHttpResponseJSON(json["cellular"].to<JsonObject>());
}

bool NetworkConfig::fromHttpRequestJSON(JsonObject json) {
    if (!json["preferredTransport"].is<String>() || !json["wifi"].is<JsonObject>() ||
        !json["cellular"].is<JsonObject>()) {
        return false;
    }
    preferredTransport = json["preferredTransport"].as<String>();
    return wifi.fromHttpRequestJSON(json["wifi"].as<JsonObject>()) &&
           cellular.fromHttpRequestJSON(json["cellular"].as<JsonObject>());
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

void SensorConfig::writeHttpResponseArray(JsonArray array, const std::vector<SensorConfig> &sensors) {
    writeArray(array, sensors);
}

bool SensorConfig::parseHttpRequestArray(JsonArray array, std::vector<SensorConfig> &sensors) {
    return parseArray(array, sensors);
}

void WundergroundPublisherConfig::toJSON(JsonObject json) const {
    json["type"] = type;
    json["enabled"] = enabled;
    json["publishIntervalSeconds"] = publishIntervalSeconds;
    if (hasIncludeHistoryWindowMinutes) {
        json["includeHistoryWindowMinutes"] = includeHistoryWindowMinutes;
    }
    json["stationId"] = stationId;
    json["apiKey"] = apiKey;
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

void WundergroundPublisherConfig::toHttpResponseJSON(JsonObject json) const {
    json["type"] = type;
    json["enabled"] = enabled;
    json["publishIntervalSeconds"] = publishIntervalSeconds;
    if (hasIncludeHistoryWindowMinutes) {
        json["includeHistoryWindowMinutes"] = includeHistoryWindowMinutes;
    }
    json["stationId"] = stationId;
    json["apiKeyConfigured"] = apiKeyConfigured;
}

bool WundergroundPublisherConfig::fromHttpRequestJSON(JsonObject json) {
    if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
        !json["publishIntervalSeconds"].is<int>() || !json["stationId"].is<String>()) {
        return false;
    }
    type = json["type"].as<String>();
    enabled = json["enabled"].as<bool>();
    publishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
    stationId = json["stationId"].as<String>();
    if (json["apiKey"].is<String>()) {
        apiKey = json["apiKey"].as<String>();
        apiKeyConfigured = !apiKey.isEmpty();
    }
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
    json["apiKey"] = apiKey;
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

void WindyPublisherConfig::toHttpResponseJSON(JsonObject json) const {
    json["type"] = type;
    json["enabled"] = enabled;
    json["publishIntervalSeconds"] = publishIntervalSeconds;
    if (hasIncludeHistoryWindowMinutes) {
        json["includeHistoryWindowMinutes"] = includeHistoryWindowMinutes;
    }
    json["stationId"] = stationId;
    json["apiKeyConfigured"] = apiKeyConfigured;
}

bool WindyPublisherConfig::fromHttpRequestJSON(JsonObject json) {
    if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
        !json["publishIntervalSeconds"].is<int>() || !json["stationId"].is<String>()) {
        return false;
    }
    type = json["type"].as<String>();
    enabled = json["enabled"].as<bool>();
    publishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
    stationId = json["stationId"].as<String>();
    if (json["apiKey"].is<String>()) {
        apiKey = json["apiKey"].as<String>();
        apiKeyConfigured = !apiKey.isEmpty();
    }
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
    json["password"] = password;
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

void MqttPublisherConfig::toHttpResponseJSON(JsonObject json) const {
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

bool MqttPublisherConfig::fromHttpRequestJSON(JsonObject json) {
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
    if (json["password"].is<String>()) {
        password = json["password"].as<String>();
        passwordConfigured = !password.isEmpty();
    }
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

void PublishersConfig::toHttpResponseJSON(JsonObject json) const {
    wunderground.toHttpResponseJSON(json["wunderground"].to<JsonObject>());
    windy.toHttpResponseJSON(json["windy"].to<JsonObject>());
    mqtt.toHttpResponseJSON(json["mqtt"].to<JsonObject>());
}

bool PublishersConfig::fromHttpRequestJSON(JsonObject json) {
    if (!json["wunderground"].is<JsonObject>() || !json["windy"].is<JsonObject>() || !json["mqtt"].is<JsonObject>()) {
        return false;
    }
    return wunderground.fromHttpRequestJSON(json["wunderground"].as<JsonObject>()) &&
           windy.fromHttpRequestJSON(json["windy"].as<JsonObject>()) &&
           mqtt.fromHttpRequestJSON(json["mqtt"].as<JsonObject>());
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
    if (!json["station"].is<JsonObject>() || !json["sampling"].is<JsonObject>() || !json["smoothing"].is<JsonObject>()
        ||
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

void DeviceConfig::toHttpResponseJSON(JsonObject json) const {
    json["schemaVersion"] = schemaVersion;
    station.toHttpResponseJSON(json["station"].to<JsonObject>());
    sampling.toHttpResponseJSON(json["sampling"].to<JsonObject>());
    smoothing.toHttpResponseJSON(json["smoothing"].to<JsonObject>());
    storage.toHttpResponseJSON(json["storage"].to<JsonObject>());
    network.toHttpResponseJSON(json["network"].to<JsonObject>());
    smsAdmin.toHttpResponseJSON(json["smsAdmin"].to<JsonObject>());
    webUi.toHttpResponseJSON(json["webUi"].to<JsonObject>());
    SensorConfig::writeHttpResponseArray(json["sensors"].to<JsonArray>(), sensors);
    publishers.toHttpResponseJSON(json["publishers"].to<JsonObject>());
}

bool DeviceConfig::fromHttpRequestJSON(JsonObject json) {
    if (!json["station"].is<JsonObject>() || !json["sampling"].is<JsonObject>() || !json["smoothing"].is<JsonObject>()
        ||
        !json["storage"].is<JsonObject>() || !json["network"].is<JsonObject>() || !json["smsAdmin"].is<JsonObject>() ||
        !json["webUi"].is<JsonObject>() || !json["sensors"].is<JsonArray>() || !json["publishers"].is<JsonObject>()) {
        return false;
    }

    schemaVersion = json["schemaVersion"] | schemaVersion;

    return station.fromHttpRequestJSON(json["station"].as<JsonObject>()) &&
           sampling.fromHttpRequestJSON(json["sampling"].as<JsonObject>()) &&
           smoothing.fromHttpRequestJSON(json["smoothing"].as<JsonObject>()) &&
           storage.fromHttpRequestJSON(json["storage"].as<JsonObject>()) &&
           network.fromHttpRequestJSON(json["network"].as<JsonObject>()) &&
           smsAdmin.fromHttpRequestJSON(json["smsAdmin"].as<JsonObject>()) &&
           webUi.fromHttpRequestJSON(json["webUi"].as<JsonObject>()) &&
           SensorConfig::parseHttpRequestArray(json["sensors"].as<JsonArray>(), sensors) &&
           publishers.fromHttpRequestJSON(json["publishers"].as<JsonObject>());
}
