#pragma once

#include "ConfigModels.h"
#include "DeviceStatus.h"
#include "OtaUploadResult.h"

struct AppState {
    DeviceConfig config;
    DeviceStatus status;
    LatestSensorReadings latestReadings;
    std::vector<WeatherSample> history;
    OtaUploadResult lastOtaUpload;
};

AppState createDefaultState();