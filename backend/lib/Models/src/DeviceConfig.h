#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"
#include "StationConfig.h"
#include "SamplingConfig.h"
#include "SmoothingConfig.h"
#include "StorageConfig.h"
#include "NetworkConfig.h"
#include "SmsAdminConfig.h"
#include "WebUiConfig.h"
#include "SensorConfig.h"
#include "PublishersConfig.h"

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

    void toJSON(JsonObject json) const {
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

    bool fromJSON(JsonObject json) {
        if (!json["station"].is<JsonObject>() || !json["sampling"].is<JsonObject>() || !json["smoothing"].is<
                JsonObject>()
            ||
            !json["storage"].is<JsonObject>() || !json["network"].is<JsonObject>() || !json["smsAdmin"].is<JsonObject>()
            ||
            !json["webUi"].is<JsonObject>() || !json["sensors"].is<JsonArray>() || !json["publishers"].is<
                JsonObject>()) {
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

    void toHttpResponseJSON(JsonObject json) const {
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

    bool fromHttpRequestJSON(JsonObject json) {
        if (!json["station"].is<JsonObject>() || !json["sampling"].is<JsonObject>() || !json["smoothing"].is<
                JsonObject>()
            ||
            !json["storage"].is<JsonObject>() || !json["network"].is<JsonObject>() || !json["smsAdmin"].is<JsonObject>()
            ||
            !json["webUi"].is<JsonObject>() || !json["sensors"].is<JsonArray>() || !json["publishers"].is<
                JsonObject>()) {
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
};
