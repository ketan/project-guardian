#pragma once

#include <DNSServer.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiMulti.h>

class MultiWiFi {
public:
    bool begin(const char *portalSsid, const char *portalPassword);

    void loop();

    bool isPortalActive() const;

private:
    static constexpr uint8_t maxNetworks = 5;

    WiFiMulti networks;
    Preferences preferences;
    WebServer portal{80};
    DNSServer dns;
    bool portalActive = false;

    void loadNetworks();

    void startPortal(const char *ssid, const char *password);

    void sendPortalPage();

    void saveNetwork(const String &ssid, const String &password);

    static String key(const char *prefix, uint8_t index);

    static String escapeHtml(const String &value);
};
