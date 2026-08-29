#pragma once

#include <Arduino.h>
#include <WiFi.h>

class HTTPClient {
public:
    bool begin(WiFiClient &, const String &url) {
        requestedUrl() = url;
        return true;
    }
    void setTimeout(uint16_t) {}
    void addHeader(const char *name, const String &value) {
        if (String(name) == "Authorization") authorization() = value;
    }
    int GET() { return nextStatusCode(); }
    void end() {}

    static void setNextStatusCode(int statusCode) { nextStatusCode() = statusCode; }
    static String lastRequestedUrl() { return requestedUrl(); }
    static String lastAuthorization() { return authorization(); }

private:
    static int &nextStatusCode() {
        static int statusCode = -1;
        return statusCode;
    }
    static String &requestedUrl() {
        static String url;
        return url;
    }
    static String &authorization() {
        static String header;
        return header;
    }
};
