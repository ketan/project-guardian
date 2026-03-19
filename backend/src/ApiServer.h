#pragma once

#include <ESPAsyncWebServer.h>
#include "AppState.h"

class ApiServer {
public:
    ApiServer(uint16_t port, AppState &state);

    void begin();


private:
    void registerRoutes();

    AsyncWebServer webServer;
    AppState &appState;
};
