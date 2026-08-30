#pragma once

#include <ArduinoJson.h>

class JsonConfig {
public:
    virtual ~JsonConfig() = default;

    virtual void toJSON(JsonObject json) const = 0;

    virtual bool fromJSON(JsonObject json) = 0;

    virtual void toHttpResponseJSON(JsonObject json) const {
        toJSON(json);
    }

    virtual bool fromHttpRequestJSON(JsonObject json) {
        return fromJSON(json);
    }
};
