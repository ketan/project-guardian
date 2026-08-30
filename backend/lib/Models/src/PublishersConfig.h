#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"
#include "WundergroundPublisherConfig.h"
#include "WindyPublisherConfig.h"
#include "MqttPublisherConfig.h"

class PublishersConfig : public JsonConfig {
public:
    WundergroundPublisherConfig wunderground;
    WindyPublisherConfig windy;
    MqttPublisherConfig mqtt;

    void toJSON(JsonObject json) const {
        wunderground.toJSON(json["wunderground"].to<JsonObject>());
        windy.toJSON(json["windy"].to<JsonObject>());
        mqtt.toJSON(json["mqtt"].to<JsonObject>());
    }

    bool fromJSON(JsonObject json) {
        if (!json["wunderground"].is<JsonObject>() || !json["windy"].is<JsonObject>() || !json["mqtt"].is<
                JsonObject>()) {
            return false;
        }
        return wunderground.fromJSON(json["wunderground"].as<JsonObject>()) &&
               windy.fromJSON(json["windy"].as<JsonObject>()) &&
               mqtt.fromJSON(json["mqtt"].as<JsonObject>());
    }

    void toHttpResponseJSON(JsonObject json) const {
        wunderground.toHttpResponseJSON(json["wunderground"].to<JsonObject>());
        windy.toHttpResponseJSON(json["windy"].to<JsonObject>());
        mqtt.toHttpResponseJSON(json["mqtt"].to<JsonObject>());
    }

    bool fromHttpRequestJSON(JsonObject json) {
        if (!json["wunderground"].is<JsonObject>() || !json["windy"].is<JsonObject>() || !json["mqtt"].is<
                JsonObject>()) {
            return false;
        }
        return wunderground.fromHttpRequestJSON(json["wunderground"].as<JsonObject>()) &&
               windy.fromHttpRequestJSON(json["windy"].as<JsonObject>()) &&
               mqtt.fromHttpRequestJSON(json["mqtt"].as<JsonObject>());
    }
};
