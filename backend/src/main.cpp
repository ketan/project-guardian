#include <Arduino.h>
#include <SEN0658.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <esp_netif.h>

#include "ApiServer.h"
#include "AppLogger.h"
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
        LOG_INFO(logger, "CPU frequency set to %u MHz", ESP.getCpuFreqMHz());
    } else {
        LOG_WARNING(logger, "CPU frequency remains at %u MHz", ESP.getCpuFreqMHz());
    }
}

String stationGlobalIPv6() {
    esp_netif_t *stationNetif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_ip6_addr_t address;
    if (stationNetif == nullptr || esp_netif_get_ip6_global(stationNetif, &address) != ESP_OK) {
        return "unavailable";
    }
    return IPv6Address(address.addr).toString();
}

void printInfoValue(Stream &output, const char *label, const String &value) {
    output.print(label);
    output.println(value);
}

template<typename Number>
void printInfoValue(Stream &output, const char *label, Number value) {
    output.print(label);
    output.println(value);
}

void printMemoryInfo(Stream &output, const char *label, uint32_t freeBytes, uint32_t totalBytes) {
    const uint32_t percentFree = totalBytes == 0 ? 0 : static_cast<uint64_t>(freeBytes) * 100 / totalBytes;
    output.print(label);
    output.print(": ");
    output.print(freeBytes);
    output.print(" / ");
    output.print(totalBytes);
    output.print(" bytes (");
    output.print(percentFree);
    output.println("% free)");
}

void printInfo(Stream &output, const char *, void *) {
    output.println("Project Guardian information");
    printInfoValue(output, "Uptime (ms): ", millis());
    printInfoValue(output, "Logger level: ", Logger::levelName(logger.level()));
    printInfoValue(output, "Wi-Fi: ", WiFi.status() == WL_CONNECTED ? "connected" : "disconnected");
    printInfoValue(output, "Station SSID: ", WiFi.SSID());
    printInfoValue(output, "Station IPv4: ", WiFi.localIP().toString());
    printInfoValue(output, "Station global IPv6: ", stationGlobalIPv6());
    printInfoValue(output, "Station link-local IPv6: ", WiFi.localIPv6().toString());
    printInfoValue(output, "Access point IPv4: ", WiFi.softAPIP().toString());
    printInfoValue(output, "Access point IPv6: ", WiFi.softAPIPv6().toString());
    printInfoValue(output, "Wi-Fi RSSI (dBm): ", static_cast<long>(WiFi.RSSI()));
    printMemoryInfo(output, "Heap", ESP.getFreeHeap(), ESP.getHeapSize());
    printInfoValue(output, "Heap low-water mark: ", ESP.getMinFreeHeap());
    printInfoValue(output, "Heap largest allocation: ", ESP.getMaxAllocHeap());
    printMemoryInfo(output, "PSRAM", ESP.getFreePsram(), ESP.getPsramSize());
    printMemoryInfo(output, "Sketch space", ESP.getFreeSketchSpace(),
                    ESP.getSketchSize() + ESP.getFreeSketchSpace());
    printInfoValue(output, "CPU frequency (MHz): ", ESP.getCpuFreqMHz());
}

void resetDevice(Stream &output, const char *, void *) {
    output.println("Restarting...");
    output.flush();
    ESP.restart();
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
        LOG_INFO(logger, "Captive portal active");
        LOG_INFO(logger, "Portal SSID: %s", manager->getConfigPortalSSID().c_str());
    });
    wifiManager.setSaveConfigCallback([]() {
        LOG_INFO(logger, "Wi-Fi credentials saved through captive portal");
    });
}

void ensureWiFiConnection() {
    LOG_INFO(logger, "Starting Wi-Fi provisioning flow");
    const bool connected = wifiManager.autoConnect(accessPointSsid, accessPointPassword);
    if (connected) {
        const bool ipv6Enabled = WiFi.enableIpV6();
        LOG_INFO(logger, "Wi-Fi connected");
        LOG_INFO(logger, "Station IPv6 enable request: %s", ipv6Enabled ? "ok" : "failed");
        LOG_INFO(logger, "Station SSID: %s", WiFi.SSID().c_str());
        LOG_INFO(logger, "Station IPv4 address: %s", WiFi.localIP().toString().c_str());
        LOG_INFO(logger, "Station IPv6 address: %s", WiFi.localIPv6().toString().c_str());
        return;
    }

    LOG_WARNING(logger, "Wi-Fi provisioning did not complete a station connection");
}

void startMdns() {
    if (!MDNS.begin(mdnsHostname)) {
        LOG_WARNING(logger, "mDNS start: failed");
        return;
    }

    MDNS.addService("http", "tcp", httpPort);
    LOG_INFO(logger, "mDNS start: ok");
    LOG_INFO(logger, "mDNS HTTP name: http://%s.local", mdnsHostname);
}

void startTelnetLoggerIfNeeded() {
    if (telnetLogger.isRunning()) {
        return;
    }

    telnetLogger.begin();
    logDestinations.add(telnetLogger);
    LOG_INFO(logger, "Telnet logger listening on port 23");
}

void onWiFiEvent(arduino_event_id_t event, arduino_event_info_t info) {
    const IPAddress ipv4NetworkAddress = calculateNetworkAddress(accessPointIp, accessPointSubnet);
    const uint8_t prefixLength = calculatePrefixLength(accessPointSubnet);

    switch (event) {
        case ARDUINO_EVENT_WIFI_AP_START:
            LOG_INFO(logger, "Access point start: ok");
            LOG_INFO(logger, "SSID: %s", accessPointSsid);
            break;

        case ARDUINO_EVENT_WIFI_AP_GOT_IP6: {
            const IPAddress portalIpv4Address = WiFi.softAPIP();
            const IPv6Address portalIpv6Address = WiFi.softAPIPv6();

            LOG_INFO(logger, "Portal IPv4 address: %s", portalIpv4Address.toString().c_str());
            if (isZeroIpv6Address(portalIpv6Address)) {
                LOG_INFO(logger, "Portal IPv6 address: pending");
            } else {
                LOG_INFO(logger, "Portal IPv6 address: %s", portalIpv6Address.toString().c_str());
            }
            LOG_INFO(logger, "IPv4 client subnet: %s/%u", ipv4NetworkAddress.toString().c_str(), prefixLength);
            break;
        }

        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            LOG_INFO(logger, "Station IPv6 enable request: %s", WiFi.enableIpV6() ? "ok" : "failed");
            LOG_INFO(logger, "Station connected to Wi-Fi: %s", WiFi.SSID().c_str());
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
            const IPv6Address stationIpv6Address = WiFi.localIPv6();
            LOG_INFO(logger, "Station IPv4 address: %s", WiFi.localIP().toString().c_str());
            break;
        }

        case ARDUINO_EVENT_WIFI_STA_GOT_IP6: {
            const IPv6Address stationIpv6Address(info.got_ip6.ip6_info.ip.addr);
            if (isZeroIpv6Address(stationIpv6Address)) {
                LOG_INFO(logger, "Station IPv6 address: pending");
            } else {
                LOG_INFO(logger, "Station IPv6 address: %s", stationIpv6Address.toString().c_str());
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
    logger.setInfoCallback(printInfo);
    logger.setResetCallback(resetDevice);

    Serial.println();
    LOG_INFO(logger, "Project Guardian starting...");
    configureLowPowerCpuFrequency();
    LOG_INFO(logger, "PSRAM available: %u bytes", ESP.getPsramSize());

    WiFi.onEvent(onWiFiEvent);
    configureWiFiManager();
    ensureWiFiConnection();
    startTelnetLoggerIfNeeded();
    startMdns();
    apiServer.begin();
    sen0658.begin();
    LOG_INFO(logger, "HTTP server listening on port %u", httpPort);
}

void loop() {
    telnetLogger.handle();
    if (telnetLogger.takeClientConnected()) {
        logger.printHelp(telnetLogger);
    }
    logger.handle(Serial, telnetLogger);
    delay(10);
}
