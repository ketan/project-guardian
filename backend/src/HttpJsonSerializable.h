#pragma once

#include <ArduinoJson.h>

class HttpJsonSerializable {
public:
    virtual ~HttpJsonSerializable() = default;

    virtual void toJSON(JsonObject json) const = 0;
};
