#include <Arduino.h>
#include <ESPmDNS.h>
#include <RemoteDebug.h>
#include <WiFi.h>

#include "ApiServer.h"
#include "state.h"

constexpr uint16_t httpPort = 8080;
constexpr const char *accessPointSsid = "guardian-admin";
constexpr const char *accessPointPassword = "guardian123";
constexpr const char *mdnsHostname = "project-guardian";

const IPAddress accessPointIp(192, 168, 50, 1);
const IPAddress accessPointGateway(192, 168, 50, 1);
const IPAddress accessPointSubnet(255, 255, 255, 0);

AppState state = createDefaultState();
ApiServer apiServer(httpPort, state);
RemoteDebug Debug;

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

void connectAccessPoint() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(accessPointIp, accessPointGateway, accessPointSubnet);
    WiFi.softAP(accessPointSsid, accessPointPassword);
    WiFi.softAPenableIpV6();
}

void logAccessPointStatus() {
    const IPAddress ipv4NetworkAddress = calculateNetworkAddress(accessPointIp, accessPointSubnet);
    const uint8_t prefixLength = calculatePrefixLength(accessPointSubnet);

    debugI("Access point start: ok");
    debugI("SSID: %s", accessPointSsid);
    debugI("Password: %s", accessPointPassword);
    debugI("IP address: %s", accessPointIp.toString().c_str());
    debugI("IPv6 address: %s", WiFi.softAPIPv6().toString().c_str());
    debugI("IPV4 Client subnet: %s/%u", ipv4NetworkAddress.toString().c_str(), prefixLength);
    debugI("API token: secret");
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

void startRemoteDebug() {
    Debug.begin(mdnsHostname);
    Debug.setSerialEnabled(true);
    Debug.setResetCmdEnabled(true);
    Debug.showTime(true);
    Debug.showColors(true);
    Debug.showDebugLevel(true);
    debugI("RemoteDebug ready");
}

void setup() {
    Serial.begin(115200);

    while (!Serial && millis() < 3000) {
        delay(10);
    }

    sleep(2000);
    Serial.println();
    Serial.println("Project Guardian starting...");

    connectAccessPoint();
    startRemoteDebug();
    logAccessPointStatus();
    startMdns();
    apiServer.begin();

    debugI("HTTP server listening on port %u", httpPort);
}

void loop() {
    Debug.handle();
    apiServer.handleClient();
    delay(2);
}
