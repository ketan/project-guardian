#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

class JsonConfig {
public:
    virtual ~JsonConfig() = default;

    virtual void toJSON(JsonObject json) const = 0;

    virtual bool fromJSON(JsonObject json) = 0;
};

class GeoLocation : public JsonConfig {
public:
    double latitude = 0.0;
    double longitude = 0.0;
    double elevationMeters = 0.0;

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
};

class NetworkConfig : public JsonConfig {
public:
    String preferredTransport = "auto";
    WifiConfig wifi;
    CellularConfig cellular;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;
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
};

class PublishersConfig : public JsonConfig {
public:
    WundergroundPublisherConfig wunderground;
    WindyPublisherConfig windy;
    MqttPublisherConfig mqtt;

    void toJSON(JsonObject json) const override;

    bool fromJSON(JsonObject json) override;
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
};

struct DeviceSummary {
    String deviceId = "guardian-kamshet-01";
    String firmwareVersion = "0.1.0";
    String hardwareModel = "ESP32-S3 + SIM7670G + SEN0658";
    unsigned long uptimeSeconds = 0;
    String currentTime = "2026-03-19T08:21:53Z";
    String lastBootReason = "power_on";
};

struct WifiStatus {
    bool enabled = true;
    bool active = true;
    bool connected = true;
    String ssid = "guardian-station";
    String ipAddress = "192.168.4.1";
    int rssiDbm = -59;
};

struct CellularStatus {
    bool enabled = true;
    bool active = false;
    bool registered = true;
    String modemType = "SIM7670G";
    String operatorName = "Airtel";
    int signalQuality = 24;
    String ipv4 = "10.111.42.9";
    String ipv6 = "2409:4043:9c2:1::9";
    String imei;
};

struct ConnectivityStatus {
    WifiStatus wifi;
    CellularStatus cellular;
};

struct StorageStatus {
    bool sdCardPresent = false;
    uint64_t freeBytes = 0;
    uint64_t usedBytes = 0;
    int retentionDays = 14;
    String oldestRecordAt = "2026-03-01T00:00:00Z";
    String newestRecordAt = "2026-03-19T08:21:30Z";
};

struct SamplingStatus {
    int intervalSeconds = 10;
    String nextSampleAt = "2026-03-19T08:22:00Z";
    String lastSampleAt = "2026-03-19T08:21:50Z";
    bool sleepEnabled = true;
    bool smoothingEnabled = true;
};

struct AdminWindowStatus {
    bool active = false;
    String openedAt;
    String expiresAt;
    String requestedBy;
};

struct SensorStatus {
    String id;
    String kind;
    bool enabled = true;
    bool healthy = true;
    String lastReadAt = "2026-03-19T08:21:50Z";
    String message = "All metrics updating on schedule";
};

struct PublisherStatus {
    String type;
    bool enabled = false;
    String lastPublishAt;
    String lastResult = "unknown";
    String message;
};

struct DeviceStatus {
    DeviceSummary device;
    ConnectivityStatus connectivity;
    StorageStatus storage;
    SamplingStatus sampling;
    AdminWindowStatus adminWindow;
    std::vector<SensorStatus> sensors;
    std::vector<PublisherStatus> publishers;
};

struct WeatherSample {
    String recordedAt = "2026-03-19T08:21:50Z";
    double temperatureC = 24.2;
    double humidityPct = 47.5;
    double pressureHpa = 1008.4;
    double windSpeedMps = 5.3;
    double windGustMps = 7.1;
    double windDirectionDeg = 245.0;
    double rainfallMm = 0.0;
    double illuminanceLux = 18450.0;
};

struct LatestSensorReadings {
    WeatherSample latest;
    WeatherSample smoothed;
    bool smoothingApplied = true;
};

struct OtaUploadResult {
    bool checksumVerified = false;
    bool staged = false;
    bool rebootScheduled = false;
    size_t firmwareSizeBytes = 0;
    String stagedPath;
    String message;
};

namespace ModelJson {
    void writeDeviceStatus(JsonObject root, const DeviceStatus &status);

    void writeLatestSensorReadings(JsonObject root, const LatestSensorReadings &readings);

    void writeHistory(JsonObject root, const std::vector<WeatherSample> &history);

    void writeOtaUploadResult(JsonObject root, const OtaUploadResult &result);
}
