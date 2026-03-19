#include "state.h"

AppState createDefaultState() {
    AppState state;

    state.config.station.stationName = "Kamshet launch";
    state.config.station.locationFromGPS = false;
    state.config.station.gpsPollIntervalHours = 6;
    state.config.station.hasLocation = true;
    state.config.station.location.latitude = 18.758;
    state.config.station.location.longitude = 73.397;
    state.config.station.location.elevationMeters = 620.0;

    state.config.network.wifi.enabled = true;
    state.config.network.wifi.ssid = "guardian-station";
    state.config.network.wifi.passwordConfigured = true;
    state.config.network.cellular.enabled = false;
    state.config.network.cellular.modemType = "SIM7670G";
    state.config.network.cellular.pinConfigured = false;

    state.config.webUi.tokenTtlMinutes = 15;

    state.config.publishers.wunderground.type = "wunderground";
    state.config.publishers.windy.type = "windy";
    state.config.publishers.mqtt.type = "mqtt";
    state.config.publishers.mqtt.brokerUrl = "mqtt://broker";
    state.config.publishers.mqtt.topic = "meshtastic/weather";
    state.config.publishers.mqtt.username = "guardian";

    state.status.device.deviceId = "guardian-kamshet-01";
    state.status.device.firmwareVersion = "0.1.0";
    state.status.device.hardwareModel = "ESP32-S3 + SIM7670G + SEN0658";
    state.status.device.currentTime = "2026-03-19T08:21:53Z";
    state.status.device.lastBootReason = "power_on";

    state.status.connectivity.wifi.enabled = true;
    state.status.connectivity.wifi.active = true;
    state.status.connectivity.wifi.connected = true;
    state.status.connectivity.wifi.ssid = "guardian-station";
    state.status.connectivity.wifi.ipAddress = "192.168.4.1";
    state.status.connectivity.wifi.rssiDbm = -59;

    state.status.connectivity.cellular.enabled = true;
    state.status.connectivity.cellular.active = false;
    state.status.connectivity.cellular.registered = true;
    state.status.connectivity.cellular.modemType = "SIM7670G";
    state.status.connectivity.cellular.operatorName = "Airtel";
    state.status.connectivity.cellular.signalQuality = 24;
    state.status.connectivity.cellular.ipv4 = "10.111.42.9";
    state.status.connectivity.cellular.ipv6 = "2409:4043:9c2:1::9";

    state.status.storage.sdCardPresent = false;
    state.status.storage.freeBytes = 0;
    state.status.storage.usedBytes = 0;
    state.status.storage.retentionDays = 14;
    state.status.storage.oldestRecordAt = "2026-03-01T00:00:00Z";
    state.status.storage.newestRecordAt = "2026-03-19T08:21:30Z";

    state.status.sampling.intervalSeconds = state.config.sampling.intervalSeconds;
    state.status.sampling.nextSampleAt = "2026-03-19T08:22:00Z";
    state.status.sampling.lastSampleAt = "2026-03-19T08:21:50Z";
    state.status.sampling.sleepEnabled = true;
    state.status.sampling.smoothingEnabled = true;

    SensorStatus sensorStatus;
    sensorStatus.id = "meteo-main";
    sensorStatus.kind = "sen0658";
    sensorStatus.enabled = true;
    sensorStatus.healthy = true;
    sensorStatus.lastReadAt = "2026-03-19T08:21:50Z";
    sensorStatus.message = "All metrics updating on schedule";
    state.status.sensors.push_back(sensorStatus);

    PublisherStatus windyStatus;
    windyStatus.type = "windy";
    windyStatus.enabled = true;
    windyStatus.lastPublishAt = "2026-03-19T08:20:00Z";
    windyStatus.lastResult = "success";
    windyStatus.message = "Published 2 minutes ago";
    state.status.publishers.push_back(windyStatus);

    PublisherStatus mqttStatus;
    mqttStatus.type = "mqtt";
    mqttStatus.enabled = true;
    mqttStatus.lastPublishAt = "2026-03-19T08:21:30Z";
    mqttStatus.lastResult = "success";
    mqttStatus.message = "Meshtastic payload bridged over MQTT";
    state.status.publishers.push_back(mqttStatus);

    WeatherSample sample;
    state.latestReadings.latest = sample;
    state.latestReadings.smoothed = sample;
    state.history.push_back(sample);

    state.lastOtaUpload.message = "No OTA upload has been staged yet.";

    return state;
}