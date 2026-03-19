#pragma once

#include <ArduinoJson.h>
#include "state.h"

void writeDeviceStatus(JsonObject root, const AppState &state);

void writeStationConfig(JsonObject root, const StationConfig &config);

void writeSamplingConfig(JsonObject root, const SamplingConfig &config);

void writeSmoothingConfig(JsonObject root, const SmoothingConfig &config);

void writeStorageConfig(JsonObject root, const StorageConfig &config);

void writeNetworkConfig(JsonObject root, const NetworkConfig &config);

void writeSmsAdminConfig(JsonObject root, const SmsAdminConfig &config);

void writeWebUiConfig(JsonObject root, const WebUiConfig &config);

void writeSensorsConfig(JsonArray root, const std::vector<SensorConfig> &sensors);

void writeWundergroundPublisher(JsonObject root, const WundergroundPublisherConfig &config);

void writeWindyPublisher(JsonObject root, const WindyPublisherConfig &config);

void writeMqttPublisher(JsonObject root, const MqttPublisherConfig &config);

void writeLatestSensorReadings(JsonObject root, const LatestSensorReadings &readings);

void writeHistory(JsonObject root, const std::vector<WeatherSample> &history);

void writeOtaUploadResult(JsonObject root, const OtaUploadResult &result);

bool parseStationConfig(JsonObject json, StationConfig &config);

bool parseSamplingConfig(JsonObject json, SamplingConfig &config);

bool parseSmoothingConfig(JsonObject json, SmoothingConfig &config);

bool parseStorageConfig(JsonObject json, StorageConfig &config);

bool parseNetworkConfig(JsonObject json, NetworkConfig &config);

bool parseSmsAdminConfig(JsonObject json, SmsAdminConfig &config);

bool parseWebUiConfig(JsonObject json, WebUiConfig &config);

bool parseSensorsConfig(JsonArray json, std::vector<SensorConfig> &sensors);

bool parseWundergroundPublisher(JsonObject json, WundergroundPublisherConfig &config);

bool parseWindyPublisher(JsonObject json, WindyPublisherConfig &config);

bool parseMqttPublisher(JsonObject json, MqttPublisherConfig &config);