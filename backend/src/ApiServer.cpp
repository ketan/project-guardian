#include "ApiServer.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <mbedtls/sha256.h>
#include "AppLogger.h"
#include "ConfigModels.h"

namespace {
    constexpr const char *bearerToken = "secret";
    constexpr const char *requestStartMsAttribute = "requestStartMs";

    class OtaUploadContext {
    public:
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

    void addCommonHeaders(AsyncWebServerResponse *response) {
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Headers", "Authorization, Content-Type");
        response->addHeader("Access-Control-Allow-Methods", "GET, PUT, POST, OPTIONS");
    }

    bool isApiV1Path(const String &path) {
        return path.startsWith("/api/v1/");
    }

    const char *toMethodName(WebRequestMethod method) {
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

    void markRequestStart(AsyncWebServerRequest *request) {
        if (!request->hasAttribute(requestStartMsAttribute)) {
            request->setAttribute(requestStartMsAttribute, static_cast<long>(millis()));
        }
    }

    void logResponse(AsyncWebServerRequest *request, int statusCode, const char *result) {
        const unsigned long startMs = static_cast<unsigned long>(
            request->getAttribute(requestStartMsAttribute, static_cast<long>(millis())));
        const unsigned long elapsedMs = millis() - startMs;
        LOG_INFO(logger, "HTTP %s %s -> %d %s (%lums)", toMethodName(request->method()), request->url().c_str(),
                 statusCode, result, elapsedMs);
    }

    void sendDocument(AsyncWebServerRequest *request, int statusCode, JsonDocument &doc, const char *result) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->setCode(statusCode);
        addCommonHeaders(response);
        serializeJsonPretty(doc, *response);
        logResponse(request, statusCode, result);
        request->send(response);
    }

    void sendError(AsyncWebServerRequest *request, int statusCode, const char *error, const String &message) {
        JsonDocument doc;
        doc["error"] = error;
        doc["message"] = message;
        sendDocument(request, statusCode, doc, error);
    }

    template<typename Callback>
    void sendJson(AsyncWebServerRequest *request, Callback callback) {
        JsonDocument doc;
        callback(doc.to<JsonObject>());
        sendDocument(request, 200, doc, "ok");
    }

    template<typename Callback>
    void sendJsonArray(AsyncWebServerRequest *request, Callback callback) {
        JsonDocument doc;
        callback(doc.to<JsonArray>());
        sendDocument(request, 200, doc, "ok");
    }

    template<typename ParseFn, typename WriteFn, typename Model, typename LogFn>
    void handlePutSection(AsyncWebServerRequest *request, JsonVariant &payload, ParseFn parseFn, WriteFn writeFn,
                          Model &model,
                          const char *label,
                          LogFn logFn) {
        if (!payload.is<JsonObject>()) {
            sendError(request, 400, "validation_error", "Expected a JSON object.");
            return;
        }

        if (!parseFn(payload.as<JsonObject>(), model)) {
            sendError(request, 400, "validation_error", String("Invalid payload for ") + label + ".");
            return;
        }

        LOG_INFO(logger, "Updated %s", label);
        logFn();
        sendJson(request, [&](JsonObject root) { writeFn(root, model); });
    }

    void handleOptions(AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(204);
        addCommonHeaders(response);
        logResponse(request, 204, "preflight");
        request->send(response);
    }
} // namespace

ApiServer::ApiServer(uint16_t port, AppState &state)
    : webServer(port), appState(state) {
    authMiddleware.setAuthType(AUTH_BEARER);
    authMiddleware.setToken(bearerToken);
}

void ApiServer::begin() {
    registerRoutes();
    webServer.begin();
}

void ApiServer::registerRoutes() {
    registerGlobalMiddleware();
    registerNotFoundRoute();
    registerStatusRoutes();
    registerStationConfigRoutes();
    registerSamplingConfigRoutes();
    registerSmoothingConfigRoutes();
    registerStorageConfigRoutes();
    registerNetworkConfigRoutes();
    registerSmsAdminConfigRoutes();
    registerWebUiConfigRoutes();
    registerSensorConfigRoutes();
    registerPublisherConfigRoutes();
    registerDataRoutes();
    registerOtaRoutes();
}

void ApiServer::registerGlobalMiddleware() {
    webServer.addMiddleware([this](AsyncWebServerRequest *request, ArMiddlewareNext next) {
        markRequestStart(request);

        if (!isApiV1Path(request->url())) {
            next();
            return;
        }

        if (request->method() == HTTP_OPTIONS) {
            handleOptions(request);
            return;
        }

        if (!authMiddleware.allowed(request)) {
            sendError(request, 401, "unauthorized", "Missing or invalid bearer token.");
            return;
        }

        next();
    });
}

void ApiServer::registerNotFoundRoute() {
    webServer.onNotFound([](AsyncWebServerRequest *request) {
        sendError(request, 404, "not_found", "Endpoint not found.");
    });
}

void ApiServer::registerStatusRoutes() {
    webServer.on("/api/v1/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        appState.status.device.uptimeSeconds = millis() / 1000;
        appState.status.connectivity.wifi.ipAddress = WiFi.softAPIP().toString();
        sendJson(request, [&](JsonObject root) { appState.status.toJSON(root); });
    });
}

void ApiServer::registerStationConfigRoutes() {
    webServer.on("/api/v1/config/station", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJson(request, [&](JsonObject root) { appState.config.station.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/station", HTTP_PUT, [this](AsyncWebServerRequest *request, JsonVariant &json) {
        handlePutSection(
            request, json,
            [](JsonObject json, StationConfig &model) { return model.fromHttpRequestJSON(json); },
            [](JsonObject root, const StationConfig &model) { model.toHttpResponseJSON(root); },
            appState.config.station,
            "station",
            [&]() {
                sendJson(request, [&](JsonObject root) { appState.config.station.toHttpResponseJSON(root); });
            });
    });
}

void ApiServer::registerSamplingConfigRoutes() {
    webServer.on("/api/v1/config/sampling", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJson(request, [&](JsonObject root) { appState.config.sampling.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/sampling", HTTP_PUT, [this](AsyncWebServerRequest *request, JsonVariant &json) {
        if (!json.is<JsonObject>()) {
            sendError(request, 400, "validation_error", "Expected a JSON object.");
            return;
        }
        if (!appState.config.sampling.fromHttpRequestJSON(json.as<JsonObject>())) {
            sendError(request, 400, "validation_error", "Invalid sampling configuration.");
            return;
        }
        appState.status.sampling.intervalSeconds = appState.config.sampling.intervalSeconds;
        LOG_INFO(logger, "Updated sampling config");
        sendJson(request, [&](JsonObject root) { appState.config.sampling.toHttpResponseJSON(root); });
    });
}

void ApiServer::registerSmoothingConfigRoutes() {
    webServer.on("/api/v1/config/smoothing", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJson(request, [&](JsonObject root) { appState.config.smoothing.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/smoothing", HTTP_PUT, [this](AsyncWebServerRequest *request, JsonVariant &json) {
        if (!json.is<JsonObject>()) {
            sendError(request, 400, "validation_error", "Expected a JSON object.");
            return;
        }
        if (!appState.config.smoothing.fromHttpRequestJSON(json.as<JsonObject>())) {
            sendError(request, 400, "validation_error", "Invalid smoothing configuration.");
            return;
        }
        appState.status.sampling.smoothingEnabled = appState.config.smoothing.enabled;
        LOG_INFO(logger, "Updated smoothing config");
        sendJson(request, [&](JsonObject root) { appState.config.smoothing.toHttpResponseJSON(root); });
    });
}

void ApiServer::registerStorageConfigRoutes() {
    webServer.on("/api/v1/config/storage", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJson(request, [&](JsonObject root) { appState.config.storage.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/storage", HTTP_PUT, [this](AsyncWebServerRequest *request, JsonVariant &json) {
        if (!json.is<JsonObject>()) {
            sendError(request, 400, "validation_error", "Expected a JSON object.");
            return;
        }
        if (!appState.config.storage.fromHttpRequestJSON(json.as<JsonObject>())) {
            sendError(request, 400, "validation_error", "Invalid storage configuration.");
            return;
        }
        appState.status.storage.retentionDays = appState.config.storage.retentionDays;
        LOG_INFO(logger, "Updated storage config");
        sendJson(request, [&](JsonObject root) { appState.config.storage.toHttpResponseJSON(root); });
    });
}

void ApiServer::registerNetworkConfigRoutes() {
    webServer.on("/api/v1/config/network", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJson(request, [&](JsonObject root) { appState.config.network.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/network", HTTP_PUT, [this](AsyncWebServerRequest *request, JsonVariant &json) {
        if (!json.is<JsonObject>()) {
            sendError(request, 400, "validation_error", "Expected a JSON object.");
            return;
        }
        if (!appState.config.network.fromHttpRequestJSON(json.as<JsonObject>())) {
            sendError(request, 400, "validation_error", "Invalid network configuration.");
            return;
        }
        appState.status.connectivity.wifi.enabled = appState.config.network.wifi.enabled;
        appState.status.connectivity.wifi.ssid = appState.config.network.wifi.ssid;
        appState.status.connectivity.cellular.enabled = appState.config.network.cellular.enabled;
        LOG_INFO(logger, "Updated network config");
        sendJson(request, [&](JsonObject root) { appState.config.network.toHttpResponseJSON(root); });
    });
}

void ApiServer::registerSmsAdminConfigRoutes() {
    webServer.on("/api/v1/config/sms-admin", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJson(request, [&](JsonObject root) { appState.config.smsAdmin.toHttpResponseJSON(root); });
    });

    webServer.on("/api/v1/config/sms-admin", HTTP_PUT, [this](AsyncWebServerRequest *request, JsonVariant &json) {
        if (!json.is<JsonObject>()) {
            sendError(request, 400, "validation_error", "Expected a JSON object.");
            return;
        }
        if (!appState.config.smsAdmin.fromHttpRequestJSON(json.as<JsonObject>())) {
            sendError(request, 400, "validation_error", "Invalid SMS admin configuration.");
            return;
        }
        LOG_INFO(logger, "Updated SMS admin config");
        sendJson(request, [&](JsonObject root) { appState.config.smsAdmin.toHttpResponseJSON(root); });
    });
}

void ApiServer::registerWebUiConfigRoutes() {
    webServer.on("/api/v1/config/web-ui", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJson(request, [&](JsonObject root) { appState.config.webUi.toHttpResponseJSON(root); });
    });
    webServer.on("/api/v1/config/web-ui", HTTP_PUT, [this](AsyncWebServerRequest *request, JsonVariant &json) {
        if (!json.is<JsonObject>()) {
            sendError(request, 400, "validation_error", "Expected a JSON object.");
            return;
        }
        if (!appState.config.webUi.fromHttpRequestJSON(json.as<JsonObject>())) {
            sendError(request, 400, "validation_error", "Invalid web UI configuration.");
            return;
        }
        LOG_INFO(logger, "Updated web UI config");
        sendJson(request, [&](JsonObject root) { appState.config.webUi.toHttpResponseJSON(root); });
    });
}

void ApiServer::registerSensorConfigRoutes() {
    webServer.on("/api/v1/config/sensors", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJsonArray(request, [&](JsonArray root) {
            SensorConfig::writeHttpResponseArray(root, appState.config.sensors);
        });
    });
    webServer.on("/api/v1/config/sensors", HTTP_PUT, [this](AsyncWebServerRequest *request, JsonVariant &json) {
        if (!json.is<JsonArray>() || !
            SensorConfig::parseHttpRequestArray(json.as<JsonArray>(), appState.config.sensors)) {
            sendError(request, 400, "validation_error", "Invalid sensor configuration.");
            return;
        }
        LOG_INFO(logger, "Updated sensors config");
        sendJsonArray(request, [&](JsonArray root) {
            SensorConfig::writeHttpResponseArray(root, appState.config.sensors);
        });
    });
}

void ApiServer::registerPublisherConfigRoutes() {
    webServer.on("/api/v1/config/publishers/wunderground", HTTP_GET,
                 [this](AsyncWebServerRequest *request) {
                     sendJson(request, [&](JsonObject root) {
                         appState.config.publishers.wunderground.toHttpResponseJSON(root);
                     });
                 });
    webServer.on("/api/v1/config/publishers/wunderground", HTTP_PUT,
                 [this](AsyncWebServerRequest *request, JsonVariant &json) {
                     if (!json.is<JsonObject>()) {
                         sendError(request, 400, "validation_error", "Expected a JSON object.");
                         return;
                     }
                     if (!appState.config.publishers.wunderground.fromHttpRequestJSON(json.as<JsonObject>())) {
                         sendError(request, 400, "validation_error",
                                   "Invalid Weather Underground publisher configuration.");
                         return;
                     }
                     LOG_INFO(logger, "Updated Weather Underground publisher config");
                     sendJson(request, [&](JsonObject root) {
                         appState.config.publishers.wunderground.toHttpResponseJSON(root);
                     });
                 });

    webServer.on("/api/v1/config/publishers/windy", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJson(request, [&](JsonObject root) { appState.config.publishers.windy.toHttpResponseJSON(root); });
    });
    webServer.on("/api/v1/config/publishers/windy", HTTP_PUT,
                 [this](AsyncWebServerRequest *request, JsonVariant &json) {
                     if (!json.is<JsonObject>()) {
                         sendError(request, 400, "validation_error", "Expected a JSON object.");
                         return;
                     }
                     if (!appState.config.publishers.windy.fromHttpRequestJSON(json.as<JsonObject>())) {
                         sendError(request, 400, "validation_error", "Invalid Windy publisher configuration.");
                         return;
                     }
                     LOG_INFO(logger, "Updated Windy publisher config");
                     sendJson(request, [&](JsonObject root) {
                         appState.config.publishers.windy.toHttpResponseJSON(root);
                     });
                 });

    webServer.on("/api/v1/config/publishers/mqtt", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJson(request, [&](JsonObject root) { appState.config.publishers.mqtt.toHttpResponseJSON(root); });
    });
    webServer.on("/api/v1/config/publishers/mqtt", HTTP_PUT, [this](AsyncWebServerRequest *request, JsonVariant &json) {
        if (!json.is<JsonObject>()) {
            sendError(request, 400, "validation_error", "Expected a JSON object.");
            return;
        }
        if (!appState.config.publishers.mqtt.fromHttpRequestJSON(json.as<JsonObject>())) {
            sendError(request, 400, "validation_error", "Invalid MQTT publisher configuration.");
            return;
        }
        LOG_INFO(logger, "Updated MQTT publisher config");
        sendJson(request, [&](JsonObject root) { appState.config.publishers.mqtt.toHttpResponseJSON(root); });
    });
}

void ApiServer::registerDataRoutes() {
    webServer.on("/api/v1/sensors/latest", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJson(request, [&](JsonObject root) { appState.latestReadings.toJSON(root); });
    });

    webServer.on("/api/v1/logs/history", HTTP_GET, [this](AsyncWebServerRequest *request) {
        sendJson(request, [&](JsonObject root) {
            WeatherSample::writeArray(root["samples"].to<JsonArray>(), appState.history);
        });
    });
}

void ApiServer::registerOtaRoutes() {
    webServer.on(
        "/api/v1/admin/ota",
        HTTP_POST,
        [this](AsyncWebServerRequest *request) {
            if (!otaUpload.checksumMatchAttempted) {
                sendError(request, 400, "validation_error", "No OTA upload data was received.");
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

            LOG_INFO(logger, "OTA upload finished");
            LOG_INFO(logger, "Provided SHA-256: %s", otaUpload.providedSha256.c_str());
            LOG_INFO(logger, "Calculated SHA-256: %s", otaUpload.calculatedSha256.c_str());
            sendJson(request, [&](JsonObject root) { appState.lastOtaUpload.toJSON(root); });
        },
        [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len,
           bool final) {
            if (index == 0) {
                otaUpload = {};
                otaUpload.active = true;
                otaUpload.providedSha256 = request->arg("sha256sum");
                mbedtls_sha256_init(&otaUpload.sha256);
                mbedtls_sha256_starts_ret(&otaUpload.sha256, 0);
                LOG_INFO(logger, "OTA upload started: %s", filename.c_str());
            }

            if (len > 0) {
                otaUpload.firmwareSizeBytes += len;
                mbedtls_sha256_update_ret(&otaUpload.sha256, data, len);
            }

            if (final) {
                uint8_t digest[32];
                mbedtls_sha256_finish_ret(&otaUpload.sha256, digest);
                mbedtls_sha256_free(&otaUpload.sha256);
                otaUpload.calculatedSha256 = bytesToHex(digest, sizeof(digest));
                otaUpload.providedSha256.toLowerCase();
                otaUpload.checksumMatchAttempted = true;
                otaUpload.checksumVerified = otaUpload.providedSha256 == otaUpload.calculatedSha256;
                otaUpload.active = false;
            }
        });
}
