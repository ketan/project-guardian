#include <Arduino.h>
#include <SEN0658.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <Logger.h>

#include "ApiServer.h"
#include "AppState.h"
#include "Tee.h"
#include "TelnetLogger.h"

constexpr uint16_t httpPort = 80;
constexpr uint32_t lowPowerCpuFrequencyMhz = 80;
constexpr const char *accessPointSsid = "guardian-admin";
constexpr const char *accessPointPassword = "guardian123";
constexpr const char *mdnsHostname = "project-guardian";

const IPAddress accessPointIp(192, 168, 50, 1);
const IPAddress accessPointGateway(192, 168, 50, 1);
const IPAddress accessPointSubnet(255, 255, 255, 0);

AppState state;
Tee logDestinations;
Logger logger(logDestinations);
ApiServer apiServer(httpPort, state);
TelnetLogger telnetLogger;
WiFiManager wifiManager;
SEN0658 sen0658(Serial1);

void startTelnetLoggerIfNeeded();

void configureLowPowerCpuFrequency() {
    if (setCpuFrequencyMhz(lowPowerCpuFrequencyMhz)) {
        INFO("CPU frequency set to %u MHz", ESP.getCpuFreqMHz());
    } else {
        WARN("CPU frequency remains at %u MHz", ESP.getCpuFreqMHz());
    }
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
        startTelnetLoggerIfNeeded();
        INFO("Captive portal active");
        INFO("Portal SSID: %s", manager->getConfigPortalSSID().c_str());
    });
    wifiManager.setSaveConfigCallback([]() {
        INFO("Wi-Fi credentials saved through captive portal");
    });
}

void ensureWiFiConnection() {
    INFO("Starting Wi-Fi provisioning flow");
    const bool connected = wifiManager.autoConnect(accessPointSsid, accessPointPassword);
    if (connected) {
        const bool ipv6Enabled = WiFi.enableIpV6();
        INFO("Wi-Fi connected");
        INFO("Station IPv6 enable request: %s", ipv6Enabled ? "ok" : "failed");
        INFO("Station SSID: %s", WiFi.SSID().c_str());
        INFO("Station IPv4 address: %s", WiFi.localIP().toString().c_str());
        INFO("Station IPv6 address: %s", WiFi.localIPv6().toString().c_str());
        return;
    }

    WARN("Wi-Fi provisioning did not complete a station connection");
}

void startMdns() {
    if (!MDNS.begin(mdnsHostname)) {
        WARN("mDNS start: failed");
        return;
    }

    MDNS.addService("http", "tcp", httpPort);
    INFO("mDNS start: ok");
    INFO("mDNS HTTP name: http://%s.local", mdnsHostname);
}

void startTelnetLoggerIfNeeded() {
    if (telnetLogger.isRunning()) {
        return;
    }

    telnetLogger.begin();
    logDestinations.add(telnetLogger);
    INFO("Telnet logger listening on port 23");
}

void onWiFiEvent(arduino_event_id_t event, arduino_event_info_t info) {
    const IPAddress ipv4NetworkAddress = calculateNetworkAddress(accessPointIp, accessPointSubnet);
    const uint8_t prefixLength = calculatePrefixLength(accessPointSubnet);

    switch (event) {
        case ARDUINO_EVENT_WIFI_AP_START:
            INFO("Access point start: ok");
            INFO("SSID: %s", accessPointSsid);
            break;

        case ARDUINO_EVENT_WIFI_AP_GOT_IP6: {
            const IPAddress portalIpv4Address = WiFi.softAPIP();
            const IPv6Address portalIpv6Address = WiFi.softAPIPv6();

            INFO("Portal IPv4 address: %s", portalIpv4Address.toString().c_str());
            if (isZeroIpv6Address(portalIpv6Address)) {
                INFO("Portal IPv6 address: pending");
            } else {
                INFO("Portal IPv6 address: %s", portalIpv6Address.toString().c_str());
            }
            INFO("IPv4 client subnet: %s/%u", ipv4NetworkAddress.toString().c_str(), prefixLength);
            break;
        }

        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            INFO("Station IPv6 enable request: %s", WiFi.enableIpV6() ? "ok" : "failed");
            INFO("Station connected to Wi-Fi: %s", WiFi.SSID().c_str());
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
            const IPv6Address stationIpv6Address = WiFi.localIPv6();
            INFO("Station IPv4 address: %s", WiFi.localIP().toString().c_str());
            break;
        }

        case ARDUINO_EVENT_WIFI_STA_GOT_IP6: {
            const IPv6Address stationIpv6Address(info.got_ip6.ip6_info.ip.addr);
            if (isZeroIpv6Address(stationIpv6Address)) {
                INFO("Station IPv6 address: pending");
            } else {
                INFO("Station IPv6 address: %s", stationIpv6Address.toString().c_str());
            }
            break;
        }

        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    logDestinations.add(Serial);

    Serial.println();
    INFO("Project Guardian starting...");
    INFO("Version: %s (%s)", VERSION, GIT_SHA);
    configureLowPowerCpuFrequency();
    INFO("PSRAM available: %u bytes", ESP.getPsramSize());

    WiFi.onEvent(onWiFiEvent);
    configureWiFiManager();
    ensureWiFiConnection();
    startTelnetLoggerIfNeeded();
    startMdns();
    apiServer.begin();
    sen0658.begin();
    INFO("HTTP server listening on port %u", httpPort);
}

void loop() {
    telnetLogger.handle();
    if (telnetLogger.takeClientConnected()) {
        logger.printHelp(telnetLogger);
    }
    logger.handle(Serial, telnetLogger);
    delay(10);
}
