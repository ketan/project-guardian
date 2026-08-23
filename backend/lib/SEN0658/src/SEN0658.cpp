#include "SEN0658.h"

#include <string.h>

SEN0658::SEN0658(HardwareSerial &serial, const uint8_t address, const SensorBaudRate baudRate)
    : serial(serial), address(address), baudRate(baudRate) {
}

void SEN0658::begin(int8_t rxPin, int8_t txPin, uint16_t timeoutMilliseconds) {
    serial.begin(static_cast<uint32_t>(baudRate), SERIAL_8N1, rxPin, txPin);
    responseTimeoutMilliseconds = timeoutMilliseconds;
}

/**
* This reads the sensor's current register values; its timeout covers only the Modbus
* transaction. PM2.5 and PM10 can take up to 90 seconds to stabilize after a change.
* These four blocks are the requests published in DFRobot's SEN0658 example.
*/
bool SEN0658::readLatestSnapshot(SEN0658Readings &readings) {
    uint8_t wind[4 * 2];
    uint8_t climate[3 * 2];
    uint8_t airQuality[3 * 2];
    uint8_t light[2 * 2];
    uint8_t rainfall[2];

    if (!readRegisterBlock(MeasurementRegister::WindSpeed, 4, wind) ||
        !readRegisterBlock(MeasurementRegister::Humidity, 3, climate) ||
        !readRegisterBlock(MeasurementRegister::PM25, 3, airQuality) ||
        !readRegisterBlock(MeasurementRegister::IlluminanceHigh, 2, light) ||
        !readRegisterBlock(MeasurementRegister::Rainfall, 1, rainfall)) {
        return false;
    }

    readings = {
        readUnsigned16(wind) / 10.0F,
        static_cast<uint8_t>(readUnsigned16(wind + 4)),
        readUnsigned16(wind + 6),
        readUnsigned16(climate) / 10.0F,
        readSigned16(climate + 2) / 10.0F,
        readUnsigned16(climate + 4) / 10.0F,
        readUnsigned16(airQuality),
        readUnsigned16(airQuality + 2),
        readUnsigned16(airQuality + 4) / 10.0F,
        static_cast<uint32_t>(readUnsigned16(light)) << 16 | readUnsigned16(light + 2),
        readUnsigned16(rainfall) / 10.0F,
    };
    error = SEN0658Error::None;
    return true;
}

bool SEN0658::setWindDirectionOffset(WindDirectionOffset offset) {
    return writeCalibrationRegister(CalibrationRegister::WindDirectionOffset, static_cast<uint16_t>(offset));
}

bool SEN0658::startWindSpeedZeroing() {
    // DFRobot specifies that the sensor completes this operation after 10 seconds.
    return writeCalibrationRegister(CalibrationRegister::WindSpeedZero, 0x00AA);
}

bool SEN0658::zeroRainfall() {
    return writeCalibrationRegister(CalibrationRegister::RainfallZero, 0x005A);
}

bool SEN0658::readAllMeasurementsInOneRequestUntested(SEN0658Readings &readings) {
    // Untested: Modbus permits this contiguous request, but DFRobot does not document it.
    uint8_t data[kAllMeasurementRegisterCount * 2];
    if (!readRegisterBlock(MeasurementRegister::Minimum, kAllMeasurementRegisterCount, data)) {
        return false;
    }

    readings = {
        readUnsigned16(data + registerOffset(MeasurementRegister::WindSpeed)) / 10.0F,
        static_cast<uint8_t>(readUnsigned16(data + registerOffset(MeasurementRegister::WindDirectionEightPoint))),
        readUnsigned16(data + registerOffset(MeasurementRegister::WindDirectionDegrees)),
        readUnsigned16(data + registerOffset(MeasurementRegister::Humidity)) / 10.0F,
        readSigned16(data + registerOffset(MeasurementRegister::Temperature)) / 10.0F,
        readUnsigned16(data + registerOffset(MeasurementRegister::Noise)) / 10.0F,
        readUnsigned16(data + registerOffset(MeasurementRegister::PM25)),
        readUnsigned16(data + registerOffset(MeasurementRegister::PM10)),
        readUnsigned16(data + registerOffset(MeasurementRegister::AtmosphericPressure)) / 10.0F,
        static_cast<uint32_t>(readUnsigned16(data + registerOffset(MeasurementRegister::IlluminanceHigh))) << 16 |
        readUnsigned16(data + registerOffset(MeasurementRegister::IlluminanceLow)),
        readUnsigned16(data + registerOffset(MeasurementRegister::Rainfall)) / 10.0F,
    };
    error = SEN0658Error::None;
    return true;
}

bool SEN0658::readRegisterBlock(MeasurementRegister firstRegister, uint8_t registerCount, uint8_t *data) {
    // Modbus function 0x03 reads one contiguous block of holding registers.
    const uint16_t firstRegisterAddress = static_cast<uint16_t>(firstRegister);
    // Request layout matches DFRobot's Com arrays:
    // [device address, function, register high, register low, count high, count low, CRC low, CRC high].
    uint8_t request[] = {
        address, // The configured Modbus device address.
        0x03, // Read holding registers.
        static_cast<uint8_t>(firstRegisterAddress >> 8), // First register, high byte.
        static_cast<uint8_t>(firstRegisterAddress), // First register, low byte.
        0, // Register count, high byte; all reads are below 256.
        registerCount, // Register count, low byte.
        0, 0, // CRC is filled in below.
    };
    const uint16_t requestCrc = crc16(request, sizeof(request) - 2);
    // Modbus RTU places the CRC low byte before its high byte.
    request[6] = static_cast<uint8_t>(requestCrc);
    request[7] = static_cast<uint8_t>(requestCrc >> 8);

    // Response = address, function, byte count, one 16-bit word per register, CRC.
    const size_t expectedResponseLength = 3 + (registerCount * 2) + 2;
    uint8_t response[3 + (kAllMeasurementRegisterCount * 2) + 2];
    SEN0658Error lastAttemptError = SEN0658Error::Timeout;
    const unsigned long startedAt = millis();

    // Like DFRobot's example, retry a request every 100 ms until the overall timeout.
    while (millis() - startedAt < responseTimeoutMilliseconds) {
        // A delayed response from an earlier request must not be decoded as this one.
        clearInput();
        serial.write(request, sizeof(request));

        size_t receivedBytes = 0;
        const unsigned long requestSentAt = millis();
        // Collect one complete response before retrying. Reading bytes individually avoids
        // blocking for the full overall timeout after a missing or corrupt response.
        while (receivedBytes < expectedResponseLength &&
               millis() - startedAt < responseTimeoutMilliseconds &&
               millis() - requestSentAt < kRequestRetryIntervalMilliseconds) {
            if (serial.available()) {
                response[receivedBytes++] = static_cast<uint8_t>(serial.read());
            }
        }

        if (receivedBytes != expectedResponseLength) {
            continue;
        }

        // Reject replies from another device, a different Modbus operation, or a partial range.
        if (response[0] != address || response[1] != 0x03 || response[2] != registerCount * 2) {
            lastAttemptError = SEN0658Error::InvalidResponse;
            continue;
        }

        const uint16_t responseCrc = crc16(response, expectedResponseLength - 2);
        // The two received CRC bytes are little-endian, unlike register values below.
        const uint16_t receivedCrc = static_cast<uint16_t>(response[expectedResponseLength - 2]) |
                                     static_cast<uint16_t>(response[expectedResponseLength - 1]) << 8;
        if (responseCrc != receivedCrc) {
            lastAttemptError = SEN0658Error::ChecksumMismatch;
            continue;
        }

        // Return only the register payload; address, function, byte count, and CRC stay internal.
        memcpy(data, response + 3, registerCount * 2);
        return true;
    }

    error = lastAttemptError;
    return false;
}

bool SEN0658::writeCalibrationRegister(CalibrationRegister reg, uint16_t value) {
    const uint16_t registerAddress = static_cast<uint16_t>(reg);
    // Modbus 0x06 writes one holding register. A successful response echoes this frame.
    uint8_t request[] = {
        address,
        0x06,
        static_cast<uint8_t>(registerAddress >> 8), static_cast<uint8_t>(registerAddress),
        static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value),
        0, 0,
    };
    const uint16_t requestCrc = crc16(request, sizeof(request) - 2);
    request[6] = static_cast<uint8_t>(requestCrc);
    request[7] = static_cast<uint8_t>(requestCrc >> 8);

    SEN0658Error lastAttemptError = SEN0658Error::Timeout;
    const unsigned long startedAt = millis();
    while (millis() - startedAt < responseTimeoutMilliseconds) {
        clearInput();
        serial.write(request, sizeof(request));

        uint8_t response[sizeof(request)];
        size_t receivedBytes = 0;
        const unsigned long requestSentAt = millis();
        while (receivedBytes < sizeof(response) &&
               millis() - startedAt < responseTimeoutMilliseconds &&
               millis() - requestSentAt < kRequestRetryIntervalMilliseconds) {
            if (serial.available()) {
                response[receivedBytes++] = static_cast<uint8_t>(serial.read());
            }
        }

        if (receivedBytes != sizeof(response)) {
            continue;
        }
        if (response[0] != address || response[1] != 0x06) {
            lastAttemptError = SEN0658Error::InvalidResponse;
            continue;
        }

        const uint16_t responseCrc = crc16(response, sizeof(response) - 2);
        const uint16_t receivedCrc = static_cast<uint16_t>(response[sizeof(response) - 2]) |
                                     static_cast<uint16_t>(response[sizeof(response) - 1]) << 8;
        if (responseCrc != receivedCrc) {
            lastAttemptError = SEN0658Error::ChecksumMismatch;
            continue;
        }
        if (memcmp(response, request, sizeof(request)) != 0) {
            lastAttemptError = SEN0658Error::InvalidResponse;
            continue;
        }

        error = SEN0658Error::None;
        return true;
    }

    error = lastAttemptError;
    return false;
}

SEN0658Error SEN0658::lastError() const {
    return error;
}

uint16_t SEN0658::crc16(const uint8_t *data, const size_t length) {
    // Standard Modbus RTU CRC-16 (initial value 0xFFFF, polynomial 0xA001).
    uint16_t crc = 0xFFFF;
    for (size_t index = 0; index < length; ++index) {
        crc ^= data[index];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = crc & 1 ? (crc >> 1) ^ 0xA001 : crc >> 1;
        }
    }
    return crc;
}

uint16_t SEN0658::readUnsigned16(const uint8_t *data) {
    // Modbus register values arrive most-significant byte first.
    return static_cast<uint16_t>(data[0]) << 8 | data[1];
}

int16_t SEN0658::readSigned16(const uint8_t *data) {
    // Temperature is the only documented signed measurement in this response.
    return static_cast<int16_t>(readUnsigned16(data));
}

void SEN0658::clearInput() {
    // Discard incomplete or delayed bytes before starting a new request/response exchange.
    while (serial.available()) {
        serial.read();
    }
}
