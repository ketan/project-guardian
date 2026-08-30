#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"
#include "SmoothingFieldConfig.h"

class SmoothingConfig : public JsonConfig {
public:
    bool enabled = true;
    std::vector<SmoothingFieldConfig> fields;

    void toJSON(JsonObject json) const {
        json["enabled"] = enabled;
        JsonArray fieldsArray = json["fields"].to<JsonArray>();
        for (const SmoothingFieldConfig &field: fields) {
            field.toJSON(fieldsArray.add<JsonObject>());
        }
    }

    bool fromJSON(JsonObject json) {
        if (!json["enabled"].is<bool>() || !json["fields"].is<JsonArray>()) {
            return false;
        }
        enabled = json["enabled"].as<bool>();
        fields.clear();
        for (JsonObject item: json["fields"].as<JsonArray>()) {
            SmoothingFieldConfig field;
            if (!field.fromJSON(item)) {
                return false;
            }
            fields.push_back(field);
        }
        return true;
    }
};
