#include "ApiServer.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <mbedtls/sha256.h>

#include "ConfigModels.h"

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
        server.setContentLength(measureJson(doc));
        server.send(statusCode, "application/json", "");
        WiFiClient client = server.client();
        serializeJson(doc, client);
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
        sendJson(webServer, [&](JsonObject root) { appState.status.toJSON(root); });
    });

    webServer.on("/api/v1/config/station", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { appState.config.station.toHttpResponseJSON(root); });
    });
    webServer.on("/api/v1/config/station", HTTP_PUT, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        handlePutSection(
            webServer,
            [](JsonObject json, StationConfig &model) { return model.fromHttpRequestJSON(json); },
            [](JsonObject root, const StationConfig &model) { model.toHttpResponseJSON(root); },
            appState.config.station,
            "station",
            [&]() {
            sendJson(webServer, [&](JsonObject root) { appState.config.station.toHttpResponseJSON(root); });
        });
    });

    webServer.on("/api/v1/config/sampling", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { appState.config.sampling.toHttpResponseJSON(root); });
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
        if (!appState.config.sampling.fromHttpRequestJSON(doc.as<JsonObject>())) {
            sendError(webServer, 400, "validation_error", "Invalid sampling configuration.");
            return;
        }
        appState.status.sampling.intervalSeconds = appState.config.sampling.intervalSeconds;
        Serial.println("Updated sampling config.");
        sendJson(webServer, [&](JsonObject root) { appState.config.sampling.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/smoothing", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { appState.config.smoothing.toHttpResponseJSON(root); });
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
        if (!appState.config.smoothing.fromHttpRequestJSON(doc.as<JsonObject>())) {
            sendError(webServer, 400, "validation_error", "Invalid smoothing configuration.");
            return;
        }
        appState.status.sampling.smoothingEnabled = appState.config.smoothing.enabled;
        Serial.println("Updated smoothing config.");
        sendJson(webServer, [&](JsonObject root) { appState.config.smoothing.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/storage", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { appState.config.storage.toHttpResponseJSON(root); });
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
        if (!appState.config.storage.fromHttpRequestJSON(doc.as<JsonObject>())) {
            sendError(webServer, 400, "validation_error", "Invalid storage configuration.");
            return;
        }
        appState.status.storage.retentionDays = appState.config.storage.retentionDays;
        Serial.println("Updated storage config.");
        sendJson(webServer, [&](JsonObject root) { appState.config.storage.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/network", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { appState.config.network.toHttpResponseJSON(root); });
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
        if (!appState.config.network.fromHttpRequestJSON(doc.as<JsonObject>())) {
            sendError(webServer, 400, "validation_error", "Invalid network configuration.");
            return;
        }
        appState.status.connectivity.wifi.enabled = appState.config.network.wifi.enabled;
        appState.status.connectivity.wifi.ssid = appState.config.network.wifi.ssid;
        appState.status.connectivity.cellular.enabled = appState.config.network.cellular.enabled;
        Serial.println("Updated network config.");
        sendJson(webServer, [&](JsonObject root) { appState.config.network.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/sms-admin", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { appState.config.smsAdmin.toHttpResponseJSON(root); });
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
        if (!appState.config.smsAdmin.fromHttpRequestJSON(doc.as<JsonObject>())) {
            sendError(webServer, 400, "validation_error", "Invalid SMS admin configuration.");
            return;
        }
        Serial.println("Updated SMS admin config.");
        sendJson(webServer, [&](JsonObject root) { appState.config.smsAdmin.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/web-ui", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { appState.config.webUi.toHttpResponseJSON(root); });
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
        if (!appState.config.webUi.fromHttpRequestJSON(doc.as<JsonObject>())) {
            sendError(webServer, 400, "validation_error", "Invalid web UI configuration.");
            return;
        }
        Serial.println("Updated web UI config.");
        sendJson(webServer, [&](JsonObject root) { appState.config.webUi.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/sensors", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJsonArray(webServer, [&](JsonArray root) { SensorConfig::writeHttpResponseArray(root, appState.config.sensors); });
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
        if (!doc.is<JsonArray>() || !SensorConfig::parseHttpRequestArray(doc.as<JsonArray>(), appState.config.sensors)) {
            sendError(webServer, 400, "validation_error", "Invalid sensor configuration.");
            return;
        }
        Serial.println("Updated sensors config.");
        sendJsonArray(webServer, [&](JsonArray root) { SensorConfig::writeHttpResponseArray(root, appState.config.sensors); });
    });

    webServer.on("/api/v1/config/publishers/wunderground", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { appState.config.publishers.wunderground.toHttpResponseJSON(root); });
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
        if (!appState.config.publishers.wunderground.fromHttpRequestJSON(doc.as<JsonObject>())) {
            sendError(webServer, 400, "validation_error", "Invalid Weather Underground publisher configuration.");
            return;
        }
        Serial.println("Updated Weather Underground publisher config.");
        sendJson(webServer, [&](JsonObject root) { appState.config.publishers.wunderground.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/publishers/windy", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { appState.config.publishers.windy.toHttpResponseJSON(root); });
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
        if (!appState.config.publishers.windy.fromHttpRequestJSON(doc.as<JsonObject>())) {
            sendError(webServer, 400, "validation_error", "Invalid Windy publisher configuration.");
            return;
        }
        Serial.println("Updated Windy publisher config.");
        sendJson(webServer, [&](JsonObject root) { appState.config.publishers.windy.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/publishers/mqtt", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { appState.config.publishers.mqtt.toHttpResponseJSON(root); });
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
        if (!appState.config.publishers.mqtt.fromHttpRequestJSON(doc.as<JsonObject>())) {
            sendError(webServer, 400, "validation_error", "Invalid MQTT publisher configuration.");
            return;
        }
        Serial.println("Updated MQTT publisher config.");
        sendJson(webServer, [&](JsonObject root) { appState.config.publishers.mqtt.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/sensors/latest", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) { appState.latestReadings.toJSON(root); });
    });

    webServer.on("/api/v1/logs/history", HTTP_GET, [this, requireAuth]() {
        if (!requireAuth()) {
            return;
        }
        sendJson(webServer, [&](JsonObject root) {
            WeatherSample::writeArray(root["samples"].to<JsonArray>(), appState.history);
        });
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
            sendJson(webServer, [&](JsonObject root) { appState.lastOtaUpload.toJSON(root); });
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
