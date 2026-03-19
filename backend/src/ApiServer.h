#pragma once

#include <WebServer.h>
#include "AppState.h"

class ApiServer {
public:
    ApiServer(uint16_t port, AppState &state);

    void begin();

    void handleClient();

private:
    void registerRoutes();

    WebServer webServer;
    AppState &appState;
};
