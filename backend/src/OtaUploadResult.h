#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "HttpJsonSerializable.h"

class OtaUploadResult : public HttpJsonSerializable {
public:
    bool checksumVerified = false;
    bool staged = false;
    bool rebootScheduled = false;
    size_t firmwareSizeBytes = 0;
    String stagedPath;
    String message;

    void toJSON(JsonObject json) const override;
};
