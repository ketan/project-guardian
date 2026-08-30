#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"

class GeoLocation : public JsonConfig {
public:
    double latitude = 0.0;
    double longitude = 0.0;
    double altitudeAboveMslMeters = 0.0;

    void toJSON(JsonObject json) const {
        json["latitude"] = latitude;
        json["longitude"] = longitude;
        json["altitudeAboveMslMeters"] = altitudeAboveMslMeters;
    }

    bool fromJSON(JsonObject json) {
        if (!json["latitude"].is<double>() || !json["longitude"].is<double>()) {
            return false;
        }

        const double parsedLatitude = json["latitude"].as<double>();
        const double parsedLongitude = json["longitude"].as<double>();
        if (!isDoubleInRange(parsedLatitude, -90.0, 90.0) || !isDoubleInRange(parsedLongitude, -180.0, 180.0)) {
            return false;
        }

        latitude = parsedLatitude;
        longitude = parsedLongitude;
        double parsedAltitudeAboveMslMeters = 0.0;
        if (json["altitudeAboveMslMeters"].is<double>()) {
            parsedAltitudeAboveMslMeters = json["altitudeAboveMslMeters"].as<double>();
        } else if (json["altitudeMslMeters"].is<double>()) {
            // Backward compatibility for earlier key names.
            parsedAltitudeAboveMslMeters = json["altitudeMslMeters"].as<double>();
        } else {
            // Backward compatibility for older payloads.
            parsedAltitudeAboveMslMeters = json["elevationMeters"] | 0.0;
        }

        if (!isDoubleInRange(parsedAltitudeAboveMslMeters, 0.0, 5000.0)) {
            return false;
        }

        altitudeAboveMslMeters = parsedAltitudeAboveMslMeters;
        return true;
    }
};
