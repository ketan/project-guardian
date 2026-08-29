import type { DeviceStatus, UiConfig } from "../api/contracts";

export const testStatus: DeviceStatus = {
  device: {
    deviceId: "guardian-kamshet-01",
    firmwareVersion: "0.1.0",
    hardwareModel: "ESP32-S3 + SIM7670G + SEN0658",
    uptimeSeconds: 86412,
    currentTime: "2026-03-19T08:21:53Z",
    lastBootReason: "timer_wakeup",
  },
  connectivity: {
    wifi: {
      enabled: true,
      active: true,
      connected: true,
      ssid: "guardian-station",
      ipAddress: "192.168.29.44",
      rssiDbm: -59,
    },
    cellular: {
      enabled: true,
      active: false,
      registered: true,
      modemType: "SIM7670G",
      operatorName: "Airtel",
      signalQuality: 24,
      ipv4: "10.111.42.9",
      ipv6: "2409:4043:9c2:1::9",
    },
  },
  storage: {
    sdCardPresent: true,
    freeBytes: 1_248_000_000,
    usedBytes: 152_000_000,
    retentionDays: 14,
    oldestRecordAt: "2026-03-01T00:00:00Z",
    newestRecordAt: "2026-03-19T08:21:30Z",
  },
  sampling: {
    intervalSeconds: 10,
    nextSampleAt: "2026-03-19T08:22:00Z",
    lastSampleAt: "2026-03-19T08:21:50Z",
    sleepEnabled: true,
    smoothingEnabled: true,
  },
  adminWindow: {
    active: false,
  },
  sensors: [
    {
      id: "meteo-main",
      kind: "sen0658",
      enabled: true,
      healthy: true,
      lastReadAt: "2026-03-19T08:21:50Z",
      message: "All metrics updating on schedule",
    },
  ],
  publishers: [
    {
      type: "windy",
      enabled: true,
      lastPublishAt: "2026-03-19T08:20:00Z",
      lastResult: "success",
      message: "Published 2 minutes ago",
    },
    {
      type: "mqtt",
      enabled: true,
      lastPublishAt: "2026-03-19T08:21:30Z",
      lastResult: "success",
      message: "Meshtastic payload bridged over MQTT",
    },
  ],
};

export const testConfig: UiConfig = {
  schemaVersion: 1,
  station: {
    stationName: "Kamshet launch",
    locationFromGPS: false,
    gpsPollIntervalHours: 6,
    location: {
      latitude: 18.758,
      longitude: 73.397,
      altitudeAboveMslMeters: 620,
    },
    notes: "",
  },
  sampling: {
    intervalSeconds: 10,
    adminWindowMinutes: 10,
    deepSleepEnabled: true,
    wakeDurationSeconds: 5,
    historyAggregationMinutes: 30,
  },
  smoothing: {
    enabled: true,
    fields: [],
  },
  storage: {
    retentionDays: 14,
    logFormat: "jsonl",
    configSource: "sd_with_flash_fallback",
  },
  network: {
    preferredTransport: "auto",
    wifi: {
      enabled: true,
      ssid: "guardian-station",
      passwordConfigured: true,
      allowedOrigins: [],
    },
    cellular: {
      enabled: false,
      modemType: "SIM7670G",
      apn: "",
      pinConfigured: false,
      smsEnabled: false,
    },
  },
  smsAdmin: {
    enabled: false,
    whitelist: [],
  },
  webUi: {
    tokenTtlMinutes: 15,
    allowedOrigins: [],
  },
  sensors: [],
  publishers: {
    wunderground: {
      type: "wunderground",
      enabled: false,
      publishIntervalSeconds: 60,
      stationId: "",
      apiKeyConfigured: false,
    },
    windy: {
      type: "windy",
      enabled: false,
      publishIntervalSeconds: 60,
      stationId: "",
      stationPasswordConfigured: false,
    },
    mqtt: {
      type: "mqtt",
      enabled: false,
      publishIntervalSeconds: 60,
      brokerUrl: "mqtt://broker",
      topic: "meshtastic/weather",
      username: "guardian",
      passwordConfigured: false,
    },
  },
};
