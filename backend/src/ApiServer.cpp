#include "ApiServer.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <mbedtls/sha256.h>

#include "json_codec.h"

namespace {
    constexpr const char *bearerToken = "secret";

    struct OtaUploadContext {
        bool active = false;
        bool checksumVerified = false;
        bool checksumMatchAttempted = false;
        size_t firmwareSizeBytes = 0;
        String providedSha256;
        String calculatedSha256;
        mbedtls_sha256_context sha256;
    };

    OtaUploadContext otaUpload;

    String bytesToHex(const uint8_t *bytes, size_t length) {
        String value;
        value.reserve(length * 2);
        for (size_t index = 0; index < length; ++index) {
            if (bytes[index] < 0x10) {
                value += "0";
            }
            value += String(bytes[index], HEX);
        }
        value.toLowerCase();
        return value;
    }

    void addCommonHeaders(WebServer &server) {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Headers", "Authorization, Content-Type");
        server.sendHeader("Access-Control-Allow-Methods", "GET, PUT, POST, OPTIONS");
    }

    bool isAuthorized(WebServer &server) {
        String header = server.header("Authorization");
        if (!header.startsWith("Bearer ")) {
            return false;
        }

        return header.substring(7) == bearerToken;
    }

    const char *toMethodName(HTTPMethod method) {
        switch (method) {
            case HTTP_GET:
                return "GET";
            case HTTP_POST:
                return "POST";
            case HTTP_PUT:
                return "PUT";
            case HTTP_PATCH:
                return "PATCH";
            case HTTP_DELETE:
                return "DELETE";
            case HTTP_OPTIONS:
                return "OPTIONS";
            default:
                return "UNKNOWN";
        }
    }

    void logRequest(WebServer &server, const char *status) {
        Serial.printf("[HTTP] %s %s -> %s\n", toMethodName(server.method()), server.uri().c_str(), status);
    }

    void sendDocument(WebServer &server, int statusCode, JsonDocument &doc) {
        addCommonHeaders(server);
        server.setContentLength(measureJsonPretty(doc));
        server.send(statusCode, "application/json", "");
        WiFiClient client = server.client();
        serializeJsonPretty(doc, client);
    }

    void sendError(WebServer &server, int statusCode, const char *error, const String &message) {
        JsonDocument doc;
        doc["error"] = error;
        doc["message"] = message;
        sendDocument(server, statusCode, doc);
    }

    template<typename Callback>
    void sendJson(WebServer &server, Callback callback) {
        JsonDocument doc;
        callback(doc.to<JsonObject>());
        sendDocument(server, 200, doc);
    }

    template<typename Callback>
    void sendJsonArray(WebServer &server, Callback callback) {
        JsonDocument doc;
        callback(doc.to<JsonArray>());
        sendDocument(server, 200, doc);
    }

    template<typename ParseFn, typename WriteFn, typename Model, typename LogFn>
    void handlePutSection(WebServer &server, ParseFn parseFn, WriteFn writeFn, Model &model, const char *label,
                          LogFn logFn) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        if (error) {
            sendError(server, 400, "validation_error", String("Invalid JSON: ") + error.c_str());
            return;
        }

        if (!doc.is<JsonObject>()) {
            sendError(server, 400, "validation_error", "Expected a JSON object.");
            return;
        }

        if (!parseFn(doc.as<JsonObject>(), model)) {
            sendError(server, 400, "validation_error", String("Invalid payload for ") + label + ".");
            return;
        }

        Serial.printf("Updated %s:\n", label);
        logFn();
        sendJson(server, [&](JsonObject root) { writeFn(root, model); });
    }

    void handleOptions(WebServer &server) {
        addCommonHeaders(server);
        server.send(204);
    }
} // namespace

ApiServer::ApiServer(uint16_t port, AppState &state)
    : webServer(port), appState(state) {
}

void ApiServer::begin() {
    registerRoutes();
    webServer.begin();
}

void ApiServer::handleClient() {
    webServer.handleClient();
}

void ApiServer::registerRoutes() {
    webServer.onNotFound([this]() {
        if (webServer.method() == HTTP_OPTIONS) {
            logRequest(webServer, "preflight");
            handleOptions(webServer);
            return;
        }
        logRequest(webServer, "not_found");
        sendError(webServer, 404, "not_found", "Endpoint not found.");
    });

    auto requireAuth = [this]() -> bool {
        logRequest(webServer, "incoming");

        if (webServer.method() == HTTP_OPTIONS) {
            handleOptions(webServer);
            return false;
        }

        if (!isAuthorized(webServer)) {
            logRequest(webServer, "unauthorized");
            sendError(webServer, 401, "unauthorized", "Missing or invalid bearer token.");
            return false;
        }

        return true;
    };

    webServer.on("/api/v1/status", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        appState.status.device.uptimeSeconds = millis() / 1000;
        appState.status.connectivity.wifi.ipAddress = WiFi.softAPIP().toString();
        sendJson(webServer, [&](JsonObject root) { writeDeviceStatus(root, appState); });
    });

    webServer.on("/api/v1/config/station", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { writeStationConfig(root, appState.config.station); });
    });
    webServer.on("/api/v1/config/station", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        handlePutSection(webServer, parseStationConfig, writeStationConfig, appState.config.station, "station", [&]() {
            sendJson(webServer, [&](JsonObject root) { writeStationConfig(root, appState.config.station); });
        });
    });

    webServer.on("/api/v1/config/sampling", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { writeSamplingConfig(root, appState.config.sampling); });
    });
    webServer.on("/api/v1/config/sampling", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        JsonDocument doc;
        if (deserializeJson(doc, webServer.arg("plain"))) {
            sendError(webServer, 400, "validation_error", "Invalid JSON.");
            return;
        }
        if (!parseSamplingConfig(doc.as<JsonObject>(), appState.config.sampling)) {
            sendError(webServer, 400, "validation_error", "Invalid sampling configuration.");
            return;
        }
        appState.status.sampling.intervalSeconds = appState.config.sampling.intervalSeconds;
        Serial.println("Updated sampling config.");
        sendJson(webServer, [&](JsonObject root) { writeSamplingConfig(root, appState.config.sampling); });
    });

    webServer.on("/api/v1/config/smoothing", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { writeSmoothingConfig(root, appState.config.smoothing); });
    });
    webServer.on("/api/v1/config/smoothing", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        JsonDocument doc;
        if (deserializeJson(doc, webServer.arg("plain"))) {
            sendError(webServer, 400, "validation_error", "Invalid JSON.");
            return;
        }
        if (!parseSmoothingConfig(doc.as<JsonObject>(), appState.config.smoothing)) {
            sendError(webServer, 400, "validation_error", "Invalid smoothing configuration.");
            return;
        }
        appState.status.sampling.smoothingEnabled = appState.config.smoothing.enabled;
        Serial.println("Updated smoothing config.");
        sendJson(webServer, [&](JsonObject root) { writeSmoothingConfig(root, appState.config.smoothing); });
    });

    webServer.on("/api/v1/config/storage", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { writeStorageConfig(root, appState.config.storage); });
    });
    webServer.on("/api/v1/config/storage", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        JsonDocument doc;
        if (deserializeJson(doc, webServer.arg("plain"))) {
            sendError(webServer, 400, "validation_error", "Invalid JSON.");
            return;
        }
        if (!parseStorageConfig(doc.as<JsonObject>(), appState.config.storage)) {
            sendError(webServer, 400, "validation_error", "Invalid storage configuration.");
            return;
        }
        appState.status.storage.retentionDays = appState.config.storage.retentionDays;
        Serial.println("Updated storage config.");
        sendJson(webServer, [&](JsonObject root) { writeStorageConfig(root, appState.config.storage); });
    });

    webServer.on("/api/v1/config/network", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { writeNetworkConfig(root, appState.config.network); });
    });
    webServer.on("/api/v1/config/network", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        JsonDocument doc;
        if (deserializeJson(doc, webServer.arg("plain"))) {
            sendError(webServer, 400, "validation_error", "Invalid JSON.");
            return;
        }
        if (!parseNetworkConfig(doc.as<JsonObject>(), appState.config.network)) {
            sendError(webServer, 400, "validation_error", "Invalid network configuration.");
            return;
        }
        appState.status.connectivity.wifi.enabled = appState.config.network.wifi.enabled;
        appState.status.connectivity.wifi.ssid = appState.config.network.wifi.ssid;
        appState.status.connectivity.cellular.enabled = appState.config.network.cellular.enabled;
        Serial.println("Updated network config.");
        sendJson(webServer, [&](JsonObject root) { writeNetworkConfig(root, appState.config.network); });
    });

    webServer.on("/api/v1/config/sms-admin", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { writeSmsAdminConfig(root, appState.config.smsAdmin); });
    });
    webServer.on("/api/v1/config/sms-admin", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        JsonDocument doc;
        if (deserializeJson(doc, webServer.arg("plain"))) {
            sendError(webServer, 400, "validation_error", "Invalid JSON.");
            return;
        }
        if (!parseSmsAdminConfig(doc.as<JsonObject>(), appState.config.smsAdmin)) {
            sendError(webServer, 400, "validation_error", "Invalid SMS admin configuration.");
            return;
        }
        Serial.println("Updated SMS admin config.");
        sendJson(webServer, [&](JsonObject root) { writeSmsAdminConfig(root, appState.config.smsAdmin); });
    });

    webServer.on("/api/v1/config/web-ui", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { writeWebUiConfig(root, appState.config.webUi); });
    });
    webServer.on("/api/v1/config/web-ui", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        JsonDocument doc;
        if (deserializeJson(doc, webServer.arg("plain"))) {
            sendError(webServer, 400, "validation_error", "Invalid JSON.");
            return;
        }
        if (!parseWebUiConfig(doc.as<JsonObject>(), appState.config.webUi)) {
            sendError(webServer, 400, "validation_error", "Invalid web UI configuration.");
            return;
        }
        Serial.println("Updated web UI config.");
        sendJson(webServer, [&](JsonObject root) { writeWebUiConfig(root, appState.config.webUi); });
    });

    webServer.on("/api/v1/config/sensors", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJsonArray(webServer, [&](JsonArray root) { writeSensorsConfig(root, appState.config.sensors); });
    });
    webServer.on("/api/v1/config/sensors", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        JsonDocument doc;
        if (deserializeJson(doc, webServer.arg("plain"))) {
            sendError(webServer, 400, "validation_error", "Invalid JSON.");
            return;
        }
        if (!doc.is<JsonArray>() || !parseSensorsConfig(doc.as<JsonArray>(), appState.config.sensors)) {
            sendError(webServer, 400, "validation_error", "Invalid sensor configuration.");
            return;
        }
        Serial.println("Updated sensors config.");
        sendJsonArray(webServer, [&](JsonArray root) { writeSensorsConfig(root, appState.config.sensors); });
    });

    webServer.on("/api/v1/config/publishers/wunderground", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) {
            writeWundergroundPublisher(root, appState.config.publishers.wunderground);
        });
    });
    webServer.on("/api/v1/config/publishers/wunderground", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        JsonDocument doc;
        if (deserializeJson(doc, webServer.arg("plain"))) {
            sendError(webServer, 400, "validation_error", "Invalid JSON.");
            return;
        }
        if (!parseWundergroundPublisher(doc.as<JsonObject>(), appState.config.publishers.wunderground)) {
            sendError(webServer, 400, "validation_error", "Invalid Weather Underground publisher configuration.");
            return;
        }
        Serial.println("Updated Weather Underground publisher config.");
        sendJson(webServer, [&](JsonObject root) {
            writeWundergroundPublisher(root, appState.config.publishers.wunderground);
        });
    });

    webServer.on("/api/v1/config/publishers/windy", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { writeWindyPublisher(root, appState.config.publishers.windy); });
    });
    webServer.on("/api/v1/config/publishers/windy", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        JsonDocument doc;
        if (deserializeJson(doc, webServer.arg("plain"))) {
            sendError(webServer, 400, "validation_error", "Invalid JSON.");
            return;
        }
        if (!parseWindyPublisher(doc.as<JsonObject>(), appState.config.publishers.windy)) {
            sendError(webServer, 400, "validation_error", "Invalid Windy publisher configuration.");
            return;
        }
        Serial.println("Updated Windy publisher config.");
        sendJson(webServer, [&](JsonObject root) { writeWindyPublisher(root, appState.config.publishers.windy); });
    });

    webServer.on("/api/v1/config/publishers/mqtt", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { writeMqttPublisher(root, appState.config.publishers.mqtt); });
    });
    webServer.on("/api/v1/config/publishers/mqtt", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        JsonDocument doc;
        if (deserializeJson(doc, webServer.arg("plain"))) {
            sendError(webServer, 400, "validation_error", "Invalid JSON.");
            return;
        }
        if (!parseMqttPublisher(doc.as<JsonObject>(), appState.config.publishers.mqtt)) {
            sendError(webServer, 400, "validation_error", "Invalid MQTT publisher configuration.");
            return;
        }
        Serial.println("Updated MQTT publisher config.");
        sendJson(webServer, [&](JsonObject root) { writeMqttPublisher(root, appState.config.publishers.mqtt); });
    });

    webServer.on("/api/v1/sensors/latest", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { writeLatestSensorReadings(root, appState.latestReadings); });
    });

    webServer.on("/api/v1/logs/history", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { writeHistory(root, appState.history); });
    });

    webServer.on(
        "/api/v1/admin/ota",
        HTTP_POST,
        [this, requireAuth]() {
            if (!requireAuth()) {
                return;
            }

            if (!otaUpload.checksumMatchAttempted) {
                sendError(webServer, 400, "validation_error", "No OTA upload data was received.");
                return;
            }

            appState.lastOtaUpload.checksumVerified = otaUpload.checksumVerified;
            appState.lastOtaUpload.firmwareSizeBytes = otaUpload.firmwareSizeBytes;
            appState.lastOtaUpload.staged = false;
            appState.lastOtaUpload.rebootScheduled = false;

            if (otaUpload.checksumVerified) {
                appState.lastOtaUpload.message = "Firmware verified in memory. SD staging is not implemented yet.";
            } else {
                appState.lastOtaUpload.message = "Firmware verification failed. Upload discarded.";
            }

            Serial.println("OTA upload finished.");
            Serial.printf("Provided SHA-256: %s\n", otaUpload.providedSha256.c_str());
            Serial.printf("Calculated SHA-256: %s\n", otaUpload.calculatedSha256.c_str());
            sendJson(webServer, [&](JsonObject root) { writeOtaUploadResult(root, appState.lastOtaUpload); });
        },
        [this]() {
            HTTPUpload &upload = webServer.upload();
            if (upload.status == UPLOAD_FILE_START) {
                otaUpload = {};
                otaUpload.active = true;
                otaUpload.providedSha256 = webServer.arg("sha256sum");
                mbedtls_sha256_init(&otaUpload.sha256);
                mbedtls_sha256_starts_ret(&otaUpload.sha256, 0);
                Serial.printf("OTA upload started: %s\n", upload.filename.c_str());
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                otaUpload.firmwareSizeBytes += upload.currentSize;
                mbedtls_sha256_update_ret(&otaUpload.sha256, upload.buf, upload.currentSize);
            } else if (upload.status == UPLOAD_FILE_END) {
                uint8_t digest[32];
                mbedtls_sha256_finish_ret(&otaUpload.sha256, digest);
                mbedtls_sha256_free(&otaUpload.sha256);
                otaUpload.calculatedSha256 = bytesToHex(digest, sizeof(digest));
                otaUpload.providedSha256.toLowerCase();
                otaUpload.checksumMatchAttempted = true;
                otaUpload.checksumVerified = otaUpload.providedSha256 == otaUpload.calculatedSha256;
                otaUpload.active = false;
            } else if (upload.status == UPLOAD_FILE_ABORTED) {
                mbedtls_sha256_free(&otaUpload.sha256);
                otaUpload = {};
            }
        });
}
