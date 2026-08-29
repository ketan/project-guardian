#pragma once

#include <memory>

#include <Arduino.h>

constexpr int WL_CONNECTED = 3;

class WiFiClass {
public:
    int status() const { return WL_CONNECTED; }
    String SSID() const { return "test-network"; }
    IPAddress localIP() const { return {}; }
    IPv6Address localIPv6() const { return IPv6Address(nullptr); }
    IPAddress softAPIP() const { return {}; }
    IPv6Address softAPIPv6() const { return IPv6Address(nullptr); }
    int RSSI() const { return 0; }
};

static WiFiClass WiFi;

class WiFiClient : public Stream {
public:
    struct State {
        bool connected = true;
        String output;
    };

    WiFiClient() = default;

    static WiFiClient connectedClient() {
        WiFiClient client;
        client.state = std::make_shared<State>();
        return client;
    }

    explicit operator bool() const { return state != nullptr && state->connected; }
    bool connected() const { return state != nullptr && state->connected; }
    void stop() {
        if (state != nullptr) {
            state->connected = false;
        }
    }

    size_t write(uint8_t byte) override { return write(&byte, 1); }
    size_t write(const uint8_t *buffer, size_t size) override {
        if (!connected()) {
            return 0;
        }
        state->output.append(reinterpret_cast<const char *>(buffer), size);
        return size;
    }

    int available() override { return 0; }
    int read() override { return -1; }
    int peek() override { return -1; }
    void flush() override {}

    String output() const { return state == nullptr ? String() : state->output; }

private:
    std::shared_ptr<State> state;
};

class WiFiServer {
public:
    explicit WiFiServer(uint16_t) {}

    void begin() {}
    WiFiClient available() {
        WiFiClient client = queuedClient();
        queuedClient() = WiFiClient();
        return client;
    }

    static void queueClient(WiFiClient client) { queuedClient() = client; }

private:
    static WiFiClient &queuedClient() {
        static WiFiClient client;
        return client;
    }
};
