#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"
#include "PhoneWhitelistEntry.h"

class SmsAdminConfig : public JsonConfig {
public:
    bool enabled = false;
    std::vector<PhoneWhitelistEntry> whitelist;

    void toJSON(JsonObject json) const {
        json["enabled"] = enabled;
        JsonArray whitelistArray = json["whitelist"].to<JsonArray>();
        for (const PhoneWhitelistEntry &entry: whitelist) {
            entry.toJSON(whitelistArray.add<JsonObject>());
        }
    }

    bool fromJSON(JsonObject json) {
        if (!json["enabled"].is<bool>() || !json["whitelist"].is<JsonArray>()) {
            return false;
        }

        if (json["whitelist"].size() > 5) {
            return false;
        }

        enabled = json["enabled"].as<bool>();
        whitelist.clear();
        for (JsonObject item: json["whitelist"].as<JsonArray>()) {
            PhoneWhitelistEntry entry;
            if (!entry.fromJSON(item)) {
                return false;
            }
            whitelist.push_back(entry);
        }
        return true;
    }
};
