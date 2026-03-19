#include "OtaUploadResult.h"

void OtaUploadResult::toJSON(JsonObject json) const {
    json["checksumVerified"] = checksumVerified;
    json["staged"] = staged;
    json["rebootScheduled"] = rebootScheduled;
    json["firmwareSizeBytes"] = firmwareSizeBytes;
    if (!stagedPath.isEmpty()) {
        json["stagedPath"] = stagedPath;
    }
    json["message"] = message;
}
