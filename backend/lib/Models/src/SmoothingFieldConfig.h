#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"

class SmoothingFieldConfig : public JsonConfig {
public:
    String metric;
    String method;
    bool hasWindowSamples = false;
    int windowSamples = 0;
    bool hasAlpha = false;
    double alpha = 0.0;

    void toJSON(JsonObject json) const {
        json["metric"] = metric;
        json["method"] = method;
        if (hasWindowSamples) {
            json["windowSamples"] = windowSamples;
        }
        if (hasAlpha) {
            json["alpha"] = alpha;
        }
    }

    bool fromJSON(JsonObject json) {
        if (!json["metric"].is<String>() || !json["method"].is<String>()) {
            return false;
        }
        const String parsedMetric = json["metric"].as<String>();
        const String parsedMethod = json["method"].as<String>();
        if (!isOneOf(parsedMetric, {
                         "windSpeed", "windGust", "windDirection", "temperature", "humidity", "pressure",
                         "rainfall", "illuminance", "pm2_5", "pm10", "noise"
                     })) {
            return false;
        }
        if (!isOneOf(parsedMethod, {"none", "moving_average", "ema"})) {
            return false;
        }

        metric = parsedMetric;
        method = parsedMethod;
        hasWindowSamples = json["windowSamples"].is<int>();
        if (hasWindowSamples) {
            const int parsedWindowSamples = json["windowSamples"].as<int>();
            if (!isIntInRange(parsedWindowSamples, 1, 120)) {
                return false;
            }
            windowSamples = parsedWindowSamples;
        } else if (!json["windowSamples"].isNull()) {
            return false;
        }
        hasAlpha = json["alpha"].is<double>();
        if (hasAlpha) {
            const double parsedAlpha = json["alpha"].as<double>();
            if (!isDoubleInRange(parsedAlpha, 0.0, 1.0)) {
                return false;
            }
            alpha = parsedAlpha;
        } else if (!json["alpha"].isNull()) {
            return false;
        }
        return true;
    }
};
