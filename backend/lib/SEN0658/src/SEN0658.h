#pragma once

#include <Arduino.h>

enum class SensorBaudRate : uint32_t {
    B1200 = 1200,
    B2400 = 2400,
    B4800 = 4800,
    B9600 = 9600,
    B19200 = 19200,
    B38400 = 38400,
    B57600 = 57600,
    B115200 = 115200,
};

enum class WindDirectionOffset : uint16_t {
    Normal = 0,
    Reversed = 1,
};

struct SEN0658Readings {
    float windSpeedMetersPerSecond;
    uint8_t windDirectionEightPoint;
    uint16_t windDirectionDegrees;
    float humidityPercent;
    float temperatureCelsius;
    float noiseDecibels;
    uint16_t pm25MicrogramsPerCubicMeter;
    uint16_t pm10MicrogramsPerCubicMeter;
    float pressureKiloPascals;
    uint32_t illuminanceLux;
    float rainfallMillimeters;
};

enum class SEN0658Error {
    None,
    Timeout,
    InvalidResponse,
    ChecksumMismatch,
};

class SEN0658 {
public:
    SEN0658(HardwareSerial &serial, uint8_t address = 0x01,
            SensorBaudRate baudRate = SensorBaudRate::B4800);

    void begin(int8_t rxPin = -1, int8_t txPin = -1, uint16_t timeoutMilliseconds = 1000);

    bool readLatestSnapshot(SEN0658Readings &readings);

    bool setWindDirectionOffset(WindDirectionOffset offset);

    bool startWindSpeedZeroing();

    bool zeroRainfall();

    SEN0658Error lastError() const;

private:
    enum class MeasurementRegister : uint16_t {
        Minimum = 0x01F4,
        WindSpeed = Minimum,
        WindDirectionEightPoint = 0x01F6,
        WindDirectionDegrees = 0x01F7,
        Humidity = 0x01F8,
        Temperature = 0x01F9,
        Noise = 0x01FA,
        PM25 = 0x01FB,
        PM10 = 0x01FC,
        AtmosphericPressure = 0x01FD,
        IlluminanceHigh = 0x01FE,
        IlluminanceLow = 0x01FF,
        Rainfall = 0x0201,
        Maximum = Rainfall,
    };

    enum class ConfigurationRegister : uint16_t {
        DeviceAddress = 0x07D0,
        BaudRate = 0x07D1,
    };

    enum class CalibrationRegister : uint16_t {
        WindDirectionOffset = 0x6000,
        WindSpeedZero = 0x6001,
        RainfallZero = 0x6002,
    };

    // The untested bulk-read path spans unused 0x01F5 and 0x0200.
    static constexpr uint16_t kAllMeasurementRegisterCount =
            static_cast<uint16_t>(MeasurementRegister::Maximum) -
            static_cast<uint16_t>(MeasurementRegister::Minimum) + 1;
    static constexpr uint16_t kRequestRetryIntervalMilliseconds = 100;

    static uint16_t crc16(const uint8_t *data, size_t length);

    static uint16_t readUnsigned16(const uint8_t *data);

    static int16_t readSigned16(const uint8_t *data);

    bool readRegisterBlock(MeasurementRegister firstRegister, uint8_t registerCount, uint8_t *data);

    bool writeCalibrationRegister(CalibrationRegister reg, uint16_t value);

    bool readAllMeasurementsInOneRequestUntested(SEN0658Readings &readings);

    // Each Modbus register is two bytes; offsets are relative to 0x01F4.
    static constexpr size_t registerOffset(MeasurementRegister reg) {
        return (static_cast<uint16_t>(reg) - static_cast<uint16_t>(MeasurementRegister::Minimum)) * 2;
    }

    void clearInput();

    HardwareSerial &serial;
    uint8_t address;
    SensorBaudRate baudRate;
    uint16_t responseTimeoutMilliseconds = 1000;
    SEN0658Error error = SEN0658Error::None;
};


typedef enum : int8_t {
    TREND_STATE_DOWN = 0,
    TREND_STATE_STEADY = 1,
    TREND_STATE_UP = 2,
} trend_state_t;
