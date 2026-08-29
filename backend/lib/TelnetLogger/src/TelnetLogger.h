#pragma once

#include <WiFi.h>

class TelnetLogger : public Stream {
public:
    explicit TelnetLogger(uint16_t port = 23) : server(port) {}

    void begin() {
        if (started) {
            return;
        }
        server.begin();
        started = true;
    }

    bool isRunning() const { return started; }

    void handle() {
        if (!started) {
            return;
        }

        WiFiClient nextClient = server.available();
        if (nextClient) {
            if (client) {
                client.stop();
            }
            client = nextClient;
            clientConnected = true;
        }

        if (client && !client.connected()) {
            client.stop();
        }
    }

    size_t write(uint8_t byte) override {
        return write(&byte, 1);
    }

    size_t write(const uint8_t *buffer, size_t size) override {
        if (!client || !client.connected()) {
            return 0;
        }
        return client.write(buffer, size);
    }

    int available() override { return client && client.connected() ? client.available() : 0; }
    int read() override { return available() > 0 ? client.read() : -1; }
    int peek() override { return available() > 0 ? client.peek() : -1; }
    void flush() override { client.flush(); }

    bool takeClientConnected() {
        const bool connected = clientConnected;
        clientConnected = false;
        return connected;
    }

private:
    WiFiServer server;
    WiFiClient client;
    bool started = false;
    bool clientConnected = false;
};
