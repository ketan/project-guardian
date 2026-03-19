#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

class JsonConfig {
public:
    virtual ~JsonConfig() = default;

    virtual void toJSON(JsonObject json) const = 0;

    virtual bool fromJSON(JsonObject json) = 0;

    virtual void toHttpResponseJSON(JsonObject json) const {
        toJSON(json);
    }

    virtual bool fromHttpRequestJSON(JsonObject json) {
        return fromJSON(json);
    }
};

class GeoLocation : public JsonConfig {
public:
    double latitude = 0.0;
    double longitude = 0.0;
    double altitudeAboveMslMeters = 0.0;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;
};

class StationConfig : public JsonConfig {
public:
    String stationName;
    bool locationFromGPS = false;
    int gpsPollIntervalHours = 6;
    bool hasLocation = false;
    GeoLocation location;
    String notes;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;
};

class SamplingConfig : public JsonConfig {
public:
    int intervalSeconds = 10;
    int adminWindowMinutes = 10;
    bool deepSleepEnabled = true;
    int wakeDurationSeconds = 5;
    int historyAggregationMinutes = 30;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;
};

class SmoothingFieldConfig : public JsonConfig {
public:
    String metric;
    String method;
    bool hasWindowSamples = false;
    int windowSamples = 0;
    bool hasAlpha = false;
    double alpha = 0.0;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;
};

class SmoothingConfig : public JsonConfig {
public:
    bool enabled = true;
    std::vector<SmoothingFieldConfig> fields;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;
};

class StorageConfig : public JsonConfig {
public:
    int retentionDays = 14;
    String logFormat = "jsonl";
    String configSource = "sd_with_flash_fallback";

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;
};

class WifiConfig : public JsonConfig {
public:
    bool enabled = true;
    String ssid = "guardian-station";
    String password;
    bool passwordConfigured = true;
    std::vector<String> allowedOrigins;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;

    void toHttpResponseJSON(JsonObject json) const override;

    bool fromHttpRequestJSON(JsonObject json) override;
};

class CellularConfig : public JsonConfig {
public:
    bool enabled = false;
    String modemType = "SIM7670G";
    String apn;
    String pin;
    bool pinConfigured = false;
    bool smsEnabled = false;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;

    void toHttpResponseJSON(JsonObject json) const override;

    bool fromHttpRequestJSON(JsonObject json) override;
};

class NetworkConfig : public JsonConfig {
public:
    String preferredTransport = "auto";
    WifiConfig wifi;
    CellularConfig cellular;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;

    void toHttpResponseJSON(JsonObject json) const override;

    bool fromHttpRequestJSON(JsonObject json) override;
};

class PhoneWhitelistEntry : public JsonConfig {
public:
    String label;
    String phoneNumber;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;
};

class SmsAdminConfig : public JsonConfig {
public:
    bool enabled = false;
    std::vector<PhoneWhitelistEntry> whitelist;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;
};

class WebUiConfig : public JsonConfig {
public:
    int tokenTtlMinutes = 15;
    std::vector<String> allowedOrigins;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;
};

class SensorConfig : public JsonConfig {
public:
    String id;
    String type;
    bool enabled = false;
    String transport;
    bool hasPollIntervalSeconds = false;
    int pollIntervalSeconds = 0;
    bool hasAddress = false;
    int address = 0;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;

    static void writeArray(JsonArray array, const std::vector<SensorConfig> &sensors);

    static bool parseArray(JsonArray array, std::vector<SensorConfig> &sensors);

    static void writeHttpResponseArray(JsonArray array, const std::vector<SensorConfig> &sensors);

    static bool parseHttpRequestArray(JsonArray array, std::vector<SensorConfig> &sensors);
};

class WundergroundPublisherConfig : public JsonConfig {
public:
    String type = "wunderground";
    bool enabled = false;
    int publishIntervalSeconds = 60;
    bool hasIncludeHistoryWindowMinutes = false;
    int includeHistoryWindowMinutes = 0;
    String stationId;
    String apiKey;
    bool apiKeyConfigured = false;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;

    void toHttpResponseJSON(JsonObject json) const override;

    bool fromHttpRequestJSON(JsonObject json) override;
};

class WindyPublisherConfig : public JsonConfig {
public:
    String type = "windy";
    bool enabled = false;
    int publishIntervalSeconds = 60;
    bool hasIncludeHistoryWindowMinutes = false;
    int includeHistoryWindowMinutes = 0;
    String stationId;
    String apiKey;
    bool apiKeyConfigured = false;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;

    void toHttpResponseJSON(JsonObject json) const override;

    bool fromHttpRequestJSON(JsonObject json) override;
};

class MqttPublisherConfig : public JsonConfig {
public:
    String type = "mqtt";
    bool enabled = false;
    int publishIntervalSeconds = 60;
    bool hasIncludeHistoryWindowMinutes = false;
    int includeHistoryWindowMinutes = 0;
    String brokerUrl = "mqtt://broker";
    String topic = "meshtastic/weather";
    String username = "guardian";
    String password;
    bool passwordConfigured = false;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;

    void toHttpResponseJSON(JsonObject json) const override;

    bool fromHttpRequestJSON(JsonObject json) override;
};

class PublishersConfig : public JsonConfig {
public:
    WundergroundPublisherConfig wunderground;
    WindyPublisherConfig windy;
    MqttPublisherConfig mqtt;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;

    void toHttpResponseJSON(JsonObject json) const override;

    bool fromHttpRequestJSON(JsonObject json) override;
};

class DeviceConfig : public JsonConfig {
public:
    int schemaVersion = 1;
    StationConfig station;
    SamplingConfig sampling;
    SmoothingConfig smoothing;
    StorageConfig storage;
    NetworkConfig network;
    SmsAdminConfig smsAdmin;
    WebUiConfig webUi;
    std::vector<SensorConfig> sensors;
    PublishersConfig publishers;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;

    void toHttpResponseJSON(JsonObject json) const override;

    bool fromHttpRequestJSON(JsonObject json) override;
};
