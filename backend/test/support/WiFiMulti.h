#pragma once

#include <WiFi.h>

class WiFiMulti {
public:
    bool addAP(const char *, const char *) { return true; }
    uint8_t run(uint32_t) { return WiFi.status(); }
};
