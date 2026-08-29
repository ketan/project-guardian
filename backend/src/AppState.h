#pragma once

#include "ConfigModels.h"
#include "DeviceStatus.h"
#include "OtaUploadResult.h"

class AppState {
public:
    AppState() {
        config.station.stationName = "Kamshet launch";
        config.station.locationFromGPS = false;
        config.station.gpsPollIntervalHours = 6;
        config.station.hasLocation = true;
        config.station.location.latitude = 18.758;
        config.station.location.longitude = 73.397;
        config.station.location.altitudeAboveMslMeters = 620.0;

        config.network.wifi.enabled = true;
        config.network.wifi.ssid = "guardian-station";
        config.network.wifi.passwordConfigured = true;
        config.network.cellular.enabled = false;
        config.network.cellular.modemType = "SIM7670G";
        config.network.cellular.pinConfigured = false;
        config.webUi.tokenTtlMinutes = 15;

        config.publishers.wunderground.type = "wunderground";
        config.publishers.windy.type = "windy";
        config.publishers.mqtt.type = "mqtt";
        config.publishers.mqtt.brokerUrl = "mqtt://broker";
        config.publishers.mqtt.topic = "meshtastic/weather";
        config.publishers.mqtt.username = "guardian";

        status.device.deviceId = "guardian-kamshet-01";
        status.device.hardwareModel = "ESP32-S3 + SIM7670G + SEN0658";
        status.device.currentTime = "2026-03-19T08:21:53Z";
        status.device.lastBootReason = "power_on";

        status.connectivity.wifi.enabled = true;
        status.connectivity.wifi.active = true;
        status.connectivity.wifi.connected = true;
        status.connectivity.wifi.ssid = "guardian-station";
        status.connectivity.wifi.ipAddress = "192.168.4.1";
        status.connectivity.wifi.rssiDbm = -59;

        status.connectivity.cellular.enabled = true;
        status.connectivity.cellular.active = false;
        status.connectivity.cellular.registered = true;
        status.connectivity.cellular.modemType = "SIM7670G";
        status.connectivity.cellular.operatorName = "Airtel";
        status.connectivity.cellular.signalQuality = 24;
        status.connectivity.cellular.ipv4 = "10.111.42.9";
        status.connectivity.cellular.ipv6 = "2409:4043:9c2:1::9";

        status.storage.sdCardPresent = false;
        status.storage.freeBytes = 0;
        status.storage.usedBytes = 0;
        status.storage.retentionDays = 14;
        status.storage.oldestRecordAt = "2026-03-01T00:00:00Z";
        status.storage.newestRecordAt = "2026-03-19T08:21:30Z";

        status.sampling.intervalSeconds = config.sampling.intervalSeconds;
        status.sampling.nextSampleAt = "2026-03-19T08:22:00Z";
        status.sampling.lastSampleAt = "2026-03-19T08:21:50Z";
        status.sampling.sleepEnabled = true;
        status.sampling.smoothingEnabled = true;

        SensorStatus sensorStatus;
        sensorStatus.id = "meteo-main";
        sensorStatus.kind = "sen0658";
        sensorStatus.enabled = true;
        sensorStatus.healthy = true;
        sensorStatus.lastReadAt = "2026-03-19T08:21:50Z";
        sensorStatus.message = "All metrics updating on schedule";
        status.sensors.push_back(sensorStatus);

        PublisherStatus windyStatus;
        windyStatus.type = "windy";
        windyStatus.enabled = true;
        windyStatus.lastPublishAt = "2026-03-19T08:20:00Z";
        windyStatus.lastResult = "success";
        windyStatus.message = "Published 2 minutes ago";
        status.publishers.push_back(windyStatus);

        PublisherStatus mqttStatus;
        mqttStatus.type = "mqtt";
        mqttStatus.enabled = true;
        mqttStatus.lastPublishAt = "2026-03-19T08:21:30Z";
        mqttStatus.lastResult = "success";
        mqttStatus.message = "Meshtastic payload bridged over MQTT";
        status.publishers.push_back(mqttStatus);

        WeatherSample sample;
        latestReadings.latest = sample;
        latestReadings.smoothed = sample;
        history.push_back(sample);

        lastOtaUpload.message = "No OTA upload has been staged yet.";
    }

    DeviceConfig config;
    DeviceStatus status;
    LatestSensorReadings latestReadings;
    std::vector<WeatherSample> history;
    OtaUploadResult lastOtaUpload;
};
