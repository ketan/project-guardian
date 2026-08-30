#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"

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

    void toJSON(JsonObject json) const {
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

    bool fromJSON(JsonObject json) {
        if (!json["id"].is<String>() || !json["type"].is<String>() || !json["enabled"].is<bool>()) {
            return false;
        }
        id = json["id"].as<String>();
        const String parsedType = json["type"].as<String>();
        if (!isOneOf(parsedType, {"sen0658", "bme280", "bmp390", "lps22hb"})) {
            return false;
        }
        type = parsedType;
        enabled = json["enabled"].as<bool>();

        if (json["transport"].is<String>()) {
            const String parsedTransport = json["transport"].as<String>();
            if (!isOneOf(parsedTransport, {"rs485_modbus", "i2c", "spi"})) {
                return false;
            }
            transport = parsedTransport;
        } else if (!json["transport"].isNull()) {
            return false;
        } else {
            transport = "";
        }

        hasPollIntervalSeconds = json["pollIntervalSeconds"].is<int>();
        if (hasPollIntervalSeconds) {
            const int parsedPollIntervalSeconds = json["pollIntervalSeconds"].as<int>();
            if (!isIntInRange(parsedPollIntervalSeconds, 5, 3600)) {
                return false;
            }
            pollIntervalSeconds = parsedPollIntervalSeconds;
        } else if (!json["pollIntervalSeconds"].isNull()) {
            return false;
        }

        hasAddress = json["address"].is<int>();
        if (hasAddress) {
            const int parsedAddress = json["address"].as<int>();
            if (!isIntInRange(parsedAddress, 1, 247)) {
                return false;
            }
            address = parsedAddress;
        } else if (!json["address"].isNull()) {
            return false;
        }

        return true;
    }

    static void writeArray(JsonArray array, const std::vector<SensorConfig> &sensors) {
        for (const SensorConfig &sensor: sensors) {
            sensor.toJSON(array.add<JsonObject>());
        }
    }

    static bool parseArray(JsonArray array, std::vector<SensorConfig> &sensors) {
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

    static void writeHttpResponseArray(JsonArray array, const std::vector<SensorConfig> &sensors) {
        writeArray(array, sensors);
    }

    static bool parseHttpRequestArray(JsonArray array, std::vector<SensorConfig> &sensors) {
        return parseArray(array, sensors);
    }
};
