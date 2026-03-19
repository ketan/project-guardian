#pragma once

#include "models.h"

struct AppState {
    DeviceConfig config;
    DeviceStatus status;
    LatestSensorReadings latestReadings;
    std::vector<WeatherSample> history;
    OtaUploadResult lastOtaUpload;
};

AppState createDefaultState();