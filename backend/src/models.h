#pragma once

#include <Arduino.h>
#include <vector>

struct GeoLocation {
    double latitude = 0.0;
    double longitude = 0.0;
    double elevationMeters = 0.0;
};

struct StationConfig {
    String stationName;
    bool locationFromGPS = false;
    int gpsPollIntervalHours = 6;
    bool hasLocation = false;
    GeoLocation location;
    String notes;
};

struct SamplingConfig {
    int intervalSeconds = 10;
    int adminWindowMinutes = 10;
    bool deepSleepEnabled = true;
    int wakeDurationSeconds = 5;
    int historyAggregationMinutes = 30;
};

struct SmoothingFieldConfig {
    String metric;
    String method;
    bool hasWindowSamples = false;
    int windowSamples = 0;
    bool hasAlpha = false;
    double alpha = 0.0;
};

struct SmoothingConfig {
    bool enabled = true;
    std::vector<SmoothingFieldConfig> fields;
};

struct StorageConfig {
    int retentionDays = 14;
    String logFormat = "jsonl";
    String configSource = "sd_with_flash_fallback";
};

struct WifiConfig {
    bool enabled = true;
    String ssid = "guardian-station";
    String password;
    bool passwordConfigured = true;
    std::vector<String> allowedOrigins;
};

struct CellularConfig {
    bool enabled = false;
    String modemType = "SIM7670G";
    String apn;
    String pin;
    bool pinConfigured = false;
    bool smsEnabled = false;
};

struct NetworkConfig {
    String preferredTransport = "auto";
    WifiConfig wifi;
    CellularConfig cellular;
};

struct PhoneWhitelistEntry {
    String label;
    String phoneNumber;
};

struct SmsAdminConfig {
    bool enabled = false;
    std::vector<PhoneWhitelistEntry> whitelist;
};

struct WebUiConfig {
    int tokenTtlMinutes = 15;
    std::vector<String> allowedOrigins;
};

struct SensorConfig {
    String id;
    String type;
    bool enabled = false;
    String transport;
    bool hasPollIntervalSeconds = false;
    int pollIntervalSeconds = 0;
    bool hasAddress = false;
    int address = 0;
};

struct WundergroundPublisherConfig {
    String type = "wunderground";
    bool enabled = false;
    int publishIntervalSeconds = 60;
    bool hasIncludeHistoryWindowMinutes = false;
    int includeHistoryWindowMinutes = 0;
    String stationId;
    String apiKey;
    bool apiKeyConfigured = false;
};

struct WindyPublisherConfig {
    String type = "windy";
    bool enabled = false;
    int publishIntervalSeconds = 60;
    bool hasIncludeHistoryWindowMinutes = false;
    int includeHistoryWindowMinutes = 0;
    String stationId;
    String apiKey;
    bool apiKeyConfigured = false;
};

struct MqttPublisherConfig {
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
};

struct PublishersConfig {
    WundergroundPublisherConfig wunderground;
    WindyPublisherConfig windy;
    MqttPublisherConfig mqtt;
};

struct DeviceConfig {
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