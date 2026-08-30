#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

class String : public std::string {
public:
    using std::string::string;
    using std::string::operator=;

    bool isEmpty() const { return empty(); }
};

constexpr uint32_t SERIAL_8N1 = 0;

class HardwareSerial {
public:
    void begin(uint32_t, uint32_t, int8_t, int8_t) {}
    size_t write(const uint8_t *, size_t size) { return size; }
    int available() const { return 0; }
    int read() { return -1; }
};

inline unsigned long millis() { return 0; }
inline void delay(unsigned long) {}

class IPAddress {
public:
    String toString() const { return "0.0.0.0"; }
};

class IPv6Address {
public:
    explicit IPv6Address(const uint32_t *) {}
    String toString() const { return "::"; }
};

class EspClass {
public:
    void restart() {}
    uint32_t getFreeHeap() const { return 0; }
    uint32_t getHeapSize() const { return 0; }
    uint32_t getMinFreeHeap() const { return 0; }
    uint32_t getMaxAllocHeap() const { return 0; }
    uint32_t getFreePsram() const { return 0; }
    uint32_t getPsramSize() const { return 0; }
    uint32_t getFreeSketchSpace() const { return 0; }
    uint32_t getSketchSize() const { return 0; }
    uint32_t getCpuFreqMHz() const { return 0; }
};

static EspClass ESP;

class Print {
public:
    virtual ~Print() = default;
    virtual size_t write(uint8_t byte) = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;

    size_t print(const char *value) {
        size_t size = 0;
        while (value[size] != '\0') {
            ++size;
        }
        return write(reinterpret_cast<const uint8_t *>(value), size);
    }
};

class Stream : public Print {
public:
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
    virtual void flush() = 0;
};
