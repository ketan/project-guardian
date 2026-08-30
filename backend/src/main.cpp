#include <Arduino.h>
#include <ArduinoJson.h>
#include <SD_MMC.h>
#include <SEN0658.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <Logger.h>
#include <esp_sleep.h>

#include "ApiServer.h"
#include "AppState.h"
#include "MultiWiFi.h"
#include "Tee.h"
#include "TelnetLogger.h"
#include "WeatherCapture.h"

constexpr uint16_t httpPort = 80;
constexpr uint32_t lowPowerCpuFrequencyMhz = 80;
constexpr const char *accessPointSsid = "guardian-admin";
constexpr const char *accessPointPassword = "guardian123";
constexpr const char *mdnsHostname = "project-guardian";
constexpr uint32_t windPollIntervalMicroseconds = 1000000;
constexpr int sdMmcClkPin = 5;
constexpr int sdMmcCmdPin = 4;
constexpr int sdMmcD0Pin = 6;
constexpr const char *weatherLogPath = "/weather.ndjson";

AppState state;
Tee logDestinations;
Logger logger(logDestinations);
ApiServer apiServer(httpPort, state);
TelnetLogger telnetLogger;
MultiWiFi wifiManager;
SEN0658 sen0658(Serial1);
WeatherCapture weatherCapture;
bool sdAvailable = false;

void startTelnetLoggerIfNeeded();

bool appendAggregate(const WeatherRecord &record) {
    if (!sdAvailable || !record.hasTimestamp()) {
        WARN("Weather aggregate skipped: %s", sdAvailable ? "clock unavailable" : "SD unavailable");
        return false;
    }

    File log = SD_MMC.open(weatherLogPath, FILE_APPEND);
    if (!log) {
        WARN("Weather log open: failed");
        sdAvailable = false;
        return false;
    }
    JsonDocument document;
    record.toJSON(document.to<JsonObject>());
    const size_t recordStart = serializeJson(document, log);
    const size_t recordEnd = log.print('\n');
    log.close();
    if (recordStart == 0 || recordEnd != 1) {
        WARN("Weather log append: failed");
        return false;
    }
    INFO("Weather aggregate appended");
    return true;
}

void beginSdCard() {
    SD_MMC.setPins(sdMmcClkPin, sdMmcCmdPin, sdMmcD0Pin);
    sdAvailable = SD_MMC.begin("/sdcard", true);
    INFO("SD card init: %s", sdAvailable ? "ok" : "unavailable");
}

void configureLowPowerCpuFrequency() {
    if (setCpuFrequencyMhz(lowPowerCpuFrequencyMhz)) {
        INFO("CPU frequency set to %u MHz", ESP.getCpuFreqMHz());
    } else {
        WARN("CPU frequency remains at %u MHz", ESP.getCpuFreqMHz());
    }
}

bool isZeroIpv6Address(const IPv6Address &address) {
    for (uint8_t index = 0; index < 16; ++index) {
        if (address[index] != 0) {
            return false;
        }
    }
    return true;
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
    const bool wifiConnected = wifiManager.begin(accessPointSsid, accessPointPassword);
    startTelnetLoggerIfNeeded();
    if (wifiConnected) {
        startMdns();
        apiServer.begin();
        INFO("HTTP server listening on port %u", httpPort);
    }
    sen0658.begin();
    beginSdCard();
}

void loop() {
    wifiManager.loop();
    telnetLogger.handle();
    if (telnetLogger.takeClientConnected()) {
        logger.printHelp(telnetLogger);
    }
    logger.handle(Serial, telnetLogger);
    WeatherRecord weatherRecord;
    if (weatherCapture.capture(sen0658, weatherRecord)) {
        appendAggregate(weatherRecord);
    }
    esp_sleep_enable_timer_wakeup(windPollIntervalMicroseconds);
    esp_light_sleep_start();
}
