#pragma once

#include <Arduino.h>
#include <cstdio>
#include <HTTPClient.h>
#include <WiFi.h>

#include "ConfigModels.h"
#include "DeviceStatus.h"
#include "Version.h"

class WindyClient {
public:
    bool publish(WiFiClient &transport, const WindyPublisherConfig &config, const WeatherSample &sample) {
        statusCode = 0;
        if (config.stationId.isEmpty() || config.stationPassword.isEmpty()) {
            return false;
        }

        HTTPClient http;
        String url = "https://stations.windy.com";
        url += requestPath(config, sample);
        if (!http.begin(transport, url)) {
            return false;
        }

        String authorization = "Bearer ";
        authorization += config.stationPassword;
        http.setTimeout(responseTimeoutMs);
        http.addHeader("Authorization", authorization);
        statusCode = http.GET();
        http.end();
        return statusCode >= 200 && statusCode < 300;
    }

    int lastStatusCode() const { return statusCode; }

    // https://stations.windy.com/api-reference
    static String requestPath(const WindyPublisherConfig &config, const WeatherSample &sample) {
        String path = "/api/v2/observation/update?id=";
        path += encode(config.stationId);
        path += "&time=";
        path += encode(sample.recordedAt);

        // wind in m/s
        appendNumber(path, "wind", sample.windSpeedMps, 1);
        // gust in m/s
        appendNumber(path, "gust", sample.windGustMps, 1);
        // wind dir in degrees - 0-360
        appendNumber(path, "winddir", sample.windDirectionDeg, 0);
        // relative humidity (%)
        appendNumber(path, "humidity", sample.humidityPct, 1);
        // pressure in hectopascals (mbar)
        appendNumber(path, "mbar", sample.pressureHpa, 1);
        // precipitation over last 1 hr
        appendNumber(path, "precip", sample.rainfallMm, 1);
        // temp (degree Celsius)
        appendNumber(path, "temp", sample.temperatureC, 1);
        path += "&softwaretype=";
        path += encode(SOFTWARE_TYPE);
        return path;
    }

private:
    static constexpr unsigned long responseTimeoutMs = 10000;

    int statusCode = 0;

    static String encode(const String &value) {
        const char *hex = "0123456789ABCDEF";
        String encoded;
        for (const char *cursor = value.c_str(); *cursor != '\0'; ++cursor) {
            const unsigned char valueByte = static_cast<unsigned char>(*cursor);
            if ((valueByte >= 'a' && valueByte <= 'z') || (valueByte >= 'A' && valueByte <= 'Z') ||
                (valueByte >= '0' && valueByte <= '9') || valueByte == '-' || valueByte == '_' || valueByte == '.') {
                encoded += static_cast<char>(valueByte);
            } else {
                encoded += '%';
                encoded += hex[valueByte >> 4];
                encoded += hex[valueByte & 0x0F];
            }
        }
        return encoded;
    }

    static void appendNumber(String &path, const char *name, double value, int decimals) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), decimals == 0 ? "&%s=%.0f" : "&%s=%.1f", name, value);
        path += buffer;
    }
};
