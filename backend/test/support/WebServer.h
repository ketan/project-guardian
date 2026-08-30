#pragma once

#include <Arduino.h>

#include <functional>

constexpr int HTTP_GET = 0;
constexpr int HTTP_POST = 1;
constexpr int HTTP_ANY = 255;

class UriGlob;

class WebServer {
public:
    explicit WebServer(uint16_t) {}
    void on(const char *, int, std::function<void()>) {}
    void on(const UriGlob &, int, std::function<void()>) {}
    void onNotFound(std::function<void()>) {}
    String arg(const char *) const { return ""; }
    void send(int, const char *, const char * = "") {}
    void send(int, const char *, const String &) {}
    void send(int) {}
    void sendHeader(const char *, const char *) {}
    void begin() {}
    void handleClient() {}
};
