#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"
#include "GeoLocation.h"

class StationConfig : public JsonConfig {
public:
    String stationName;
    bool locationFromGPS = false;
    int gpsPollIntervalHours = 6;
    bool hasLocation = false;
    GeoLocation location;
    String notes;

    void toJSON(JsonObject json) const {
        json["stationName"] = stationName;
        json["locationFromGPS"] = locationFromGPS;
        json["gpsPollIntervalHours"] = gpsPollIntervalHours;
        if (hasLocation) {
            location.toJSON(json["location"].to<JsonObject>());
        }
        json["notes"] = notes;
    }

    bool fromJSON(JsonObject json) {
        if (!json["stationName"].is<String>() || !json["locationFromGPS"].is<bool>() ||
            !json["gpsPollIntervalHours"].is<int>()) {
            return false;
        }

        stationName = json["stationName"].as<String>();
        locationFromGPS = json["locationFromGPS"].as<bool>();
        const int parsedGpsPollIntervalHours = json["gpsPollIntervalHours"].as<int>();
        if (!isIntInRange(parsedGpsPollIntervalHours, 1, 24)) {
            return false;
        }
        gpsPollIntervalHours = parsedGpsPollIntervalHours;
        notes = json["notes"] | "";

        if (json["location"].is<JsonObject>()) {
            hasLocation = location.fromJSON(json["location"].as<JsonObject>());
        } else {
            hasLocation = false;
        }
        return hasLocation || !json["location"].is<JsonObject>();
    }
};
