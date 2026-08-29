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

inline unsigned long millis() { return 0; }

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
