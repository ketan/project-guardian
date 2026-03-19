#include <Arduino.h>
#include <ESPmDNS.h>
#include <RemoteDebug.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "ApiServer.h"
#include "AppState.h"

constexpr uint16_t httpPort = 80;
constexpr const char *accessPointSsid = "guardian-admin";
constexpr const char *accessPointPassword = "guardian123";
constexpr const char *mdnsHostname = "project-guardian";

const IPAddress accessPointIp(192, 168, 50, 1);
const IPAddress accessPointGateway(192, 168, 50, 1);
const IPAddress accessPointSubnet(255, 255, 255, 0);

AppState state;
ApiServer apiServer(httpPort, state);
RemoteDebug Debug;
WiFiManager wifiManager;
bool remoteDebugStarted = false;

void startRemoteDebugIfNeeded();

void logSerialInfo(const char *format, ...) {
    char buffer[256];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Serial.println(buffer);
}

IPAddress calculateNetworkAddress(const IPAddress &address, const IPAddress &subnetMask) {
    IPAddress networkAddress;
    for (uint8_t index = 0; index < 4; ++index) {
        networkAddress[index] = address[index] & subnetMask[index];
    }
    return networkAddress;
}

uint8_t calculatePrefixLength(const IPAddress &subnetMask) {
    uint8_t prefixLength = 0;
    for (uint8_t index = 0; index < 4; ++index) {
        uint8_t octet = subnetMask[index];
        while (octet != 0) {
            prefixLength += octet & 0x01;
            octet >>= 1;
        }
    }
    return prefixLength;
}

bool isZeroIpv6Address(const IPv6Address &address) {
    for (uint8_t index = 0; index < 16; ++index) {
        if (address[index] != 0) {
            return false;
        }
    }
    return true;
}

void configureWiFiManager() {
    wifiManager.setHostname(mdnsHostname);
    wifiManager.setAPStaticIPConfig(accessPointIp, accessPointGateway, accessPointSubnet);
    wifiManager.setShowStaticFields(false);
    wifiManager.setShowDnsFields(false);
    wifiManager.setCaptivePortalEnable(true);
    wifiManager.setAPCallback([](WiFiManager *manager) {
        WiFi.softAPenableIpV6();
        startRemoteDebugIfNeeded();
        logSerialInfo("Captive portal active");
        logSerialInfo("Portal SSID: %s", manager->getConfigPortalSSID().c_str());
        logSerialInfo("Password: %s", accessPointPassword);
        logSerialInfo("API token: secret");
    });
    wifiManager.setSaveConfigCallback([]() {
        logSerialInfo("Wi-Fi credentials saved through captive portal");
    });
}

void ensureWiFiConnection() {
    logSerialInfo("Starting Wi-Fi provisioning flow");
    const bool connected = wifiManager.autoConnect(accessPointSsid, accessPointPassword);
    if (connected) {
        const bool ipv6Enabled = WiFi.enableIpV6();
        logSerialInfo("Wi-Fi connected");
        logSerialInfo("Station IPv6 enable request: %s", ipv6Enabled ? "ok" : "failed");
        logSerialInfo("Station SSID: %s", WiFi.SSID().c_str());
        logSerialInfo("Station IPv4 address: %s", WiFi.localIP().toString().c_str());
        logSerialInfo("Station IPv6 address: %s", WiFi.localIPv6().toString().c_str());
        return;
    }

    logSerialInfo("Wi-Fi provisioning did not complete a station connection");
}

void startMdns() {
    if (!MDNS.begin(mdnsHostname)) {
        debugW("mDNS start: failed");
        return;
    }

    MDNS.addService("http", "tcp", httpPort);
    debugI("mDNS start: ok");
    debugI("mDNS HTTP name: http://%s.local", mdnsHostname);
}

void startRemoteDebugIfNeeded() {
    if (remoteDebugStarted) {
        return;
    }

    Debug.begin(mdnsHostname);
    Debug.setSerialEnabled(true);
    Debug.setResetCmdEnabled(true);
    Debug.showTime(true);
    Debug.showColors(true);
    Debug.showDebugLevel(true);
    remoteDebugStarted = true;
    debugI("RemoteDebug ready");
}

void onWiFiEvent(arduino_event_id_t event) {
    const IPAddress ipv4NetworkAddress = calculateNetworkAddress(accessPointIp, accessPointSubnet);
    const uint8_t prefixLength = calculatePrefixLength(accessPointSubnet);

    switch (event) {
        case ARDUINO_EVENT_WIFI_AP_START:
            logSerialInfo("Access point start: ok");
            logSerialInfo("SSID: %s", accessPointSsid);
            break;

        case ARDUINO_EVENT_WIFI_AP_GOT_IP6: {
            const IPAddress portalIpv4Address = WiFi.softAPIP();
            const IPv6Address portalIpv6Address = WiFi.softAPIPv6();

            logSerialInfo("Portal IPv4 address: %s", portalIpv4Address.toString().c_str());
            if (isZeroIpv6Address(portalIpv6Address)) {
                logSerialInfo("Portal IPv6 address: pending");
            } else {
                logSerialInfo("Portal IPv6 address: %s", portalIpv6Address.toString().c_str());
            }
            logSerialInfo("IPv4 client subnet: %s/%u", ipv4NetworkAddress.toString().c_str(), prefixLength);
            break;
        }

        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            logSerialInfo("Station IPv6 enable request: %s", WiFi.enableIpV6() ? "ok" : "failed");
            logSerialInfo("Station connected to Wi-Fi: %s", WiFi.SSID().c_str());
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
            const IPv6Address stationIpv6Address = WiFi.localIPv6();
            logSerialInfo("Station IPv4 address: %s", WiFi.localIP().toString().c_str());
            if (isZeroIpv6Address(stationIpv6Address)) {
                logSerialInfo("Station IPv6 address: pending");
            } else {
                logSerialInfo("Station IPv6 address: %s", stationIpv6Address.toString().c_str());
            }
            break;
        }

        case ARDUINO_EVENT_WIFI_STA_GOT_IP6: {
            const IPv6Address stationIpv6Address = WiFi.localIPv6();
            logSerialInfo("Station IPv4 address: %s", WiFi.localIP().toString().c_str());
            if (isZeroIpv6Address(stationIpv6Address)) {
                logSerialInfo("Station IPv6 address: pending");
            } else {
                logSerialInfo("Station IPv6 address: %s", stationIpv6Address.toString().c_str());
            }
            break;
        }

        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);

    delay(2000);

    Serial.println();
    Serial.println("Project Guardian starting...");

    WiFi.onEvent(onWiFiEvent);
    configureWiFiManager();
    ensureWiFiConnection();
    startRemoteDebugIfNeeded();
    startMdns();
    apiServer.begin();

    debugI("HTTP server listening on port %u", httpPort);
}

void loop() {
    Debug.handle();
    delay(2);
}
