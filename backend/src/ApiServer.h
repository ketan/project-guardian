#pragma once

#include <ESPAsyncWebServer.h>
#include "AppState.h"

class ApiServer {
public:
    ApiServer(uint16_t port, AppState &state);

    void begin();


private:
    void registerRoutes();
    void registerGlobalMiddleware();
    void registerNotFoundRoute();
    void registerStatusRoutes();
    void registerStationConfigRoutes();
    void registerSamplingConfigRoutes();
    void registerSmoothingConfigRoutes();
    void registerStorageConfigRoutes();
    void registerNetworkConfigRoutes();
    void registerSmsAdminConfigRoutes();
    void registerWebUiConfigRoutes();
    void registerSensorConfigRoutes();
    void registerPublisherConfigRoutes();
    void registerDataRoutes();
    void registerOtaRoutes();

    AsyncWebServer webServer;
    AsyncAuthenticationMiddleware authMiddleware;
    AppState &appState;
};
