import {
  FiActivity,
  FiMapPin,
  FiMessageSquare,
  FiSend,
  FiSliders,
  FiWifi,
} from "react-icons/fi";
import type { DeviceStatus, NavItem, UiConfig } from "../types/ui";

export const navItems: NavItem[] = [
  { id: "overview", label: "Overview", icon: <FiActivity /> },
  { id: "station", label: "Station", icon: <FiMapPin /> },
  { id: "sampling", label: "Sampling", icon: <FiSliders /> },
  { id: "network", label: "Network", icon: <FiWifi /> },
  { id: "publishers", label: "Publishers", icon: <FiSend /> },
  { id: "sms", label: "SMS Admin", icon: <FiMessageSquare /> },
];

export const initialConfig: UiConfig = {
  schemaVersion: 1,
  station: {
    stationName: "Kamshet Ridge Station",
    timezone: "Asia/Kolkata",
    location: {
      latitude: 18.758,
      longitude: 73.536,
      elevationMeters: 670,
    },
    notes: "Mobile-first admin console scaffold using generated OpenAPI types.",
  },
  sampling: {
    intervalSeconds: 30,
    adminWindowMinutes: 10,
    deepSleepEnabled: true,
    wakeDurationSeconds: 8,
    historyAggregationMinutes: 30,
  },
  smoothing: {
    enabled: true,
    fields: [
      { metric: "windSpeed", method: "moving_average", windowSamples: 5 },
      { metric: "windGust", method: "none" },
      { metric: "pressure", method: "ema", alpha: 0.25 },
    ],
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
      allowedOrigins: ["https://example.github.io", "http://localhost:5173"],
    },
    cellular: {
      enabled: true,
      modemType: "SIM7670G",
      apn: "internet",
      pinConfigured: false,
      smsEnabled: true,
    },
  },
  smsAdmin: {
    enabled: true,
    whitelist: [
      { label: "Primary pilot", phoneNumber: "+919812345678" },
      { label: "Backup retrieve driver", phoneNumber: "+919876543210" },
    ],
    commands: ["OPEN_SESAME", "STATUS", "HELP", "NET", "PUBLISH_NOW"],
  },
  webUi: {
    tokenTtlMinutes: 15,
    allowedOrigins: ["https://example.github.io", "http://localhost:5173"],
  },
  sensors: [
    {
      id: "meteo-main",
      type: "sen0658",
      enabled: true,
      transport: "rs485_modbus",
      settings: { address: 1, pollIntervalSeconds: 30 },
    },
  ],
  publishers: [
    {
      id: "windy-main",
      type: "windy",
      enabled: true,
      publishIntervalSeconds: 300,
      stationId: "windy-kamshet",
      apiKeyConfigured: true,
    },
    {
      id: "wunderground-main",
      type: "wunderground",
      enabled: false,
      publishIntervalSeconds: 300,
      stationId: "IKAMSHET1",
      apiKeyConfigured: false,
    },
    {
      id: "meshtastic-mqtt",
      type: "mqtt",
      enabled: true,
      publishIntervalSeconds: 60,
      brokerUrl: "mqtts://mesh.example.net:8883",
      topic: "project-guardian/weather/kamshet",
      username: "guardian",
      passwordConfigured: true,
    },
  ],
};

export const initialStatus: DeviceStatus = {
  device: {
    deviceId: "guardian-kamshet-01",
    firmwareVersion: "0.1.0",
    hardwareModel: "ESP32-S3 + SIM7670G + SEN0658",
    uptimeSeconds: 86412,
    currentTime: "2026-03-15T16:20:00Z",
    lastBootReason: "timer_wakeup",
  },
  connectivity: {
    activeTransport: "wifi",
    wifi: {
      enabled: true,
      connected: true,
      ssid: "guardian-station",
      ipAddress: "192.168.29.44",
      rssiDbm: -59,
    },
    cellular: {
      enabled: true,
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
    newestRecordAt: "2026-03-15T16:19:30Z",
  },
  sampling: {
    intervalSeconds: 30,
    nextSampleAt: "2026-03-15T16:20:30Z",
    lastSampleAt: "2026-03-15T16:20:00Z",
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
      lastReadAt: "2026-03-15T16:20:00Z",
      message: "All metrics updating on schedule",
    },
  ],
  publishers: [
    {
      id: "windy-main",
      type: "windy",
      enabled: true,
      lastPublishAt: "2026-03-15T16:15:00Z",
      lastResult: "success",
      message: "Published 5 minutes ago",
    },
    {
      id: "meshtastic-mqtt",
      type: "mqtt",
      enabled: true,
      lastPublishAt: "2026-03-15T16:19:30Z",
      lastResult: "success",
      message: "Meshtastic payload bridged over MQTT",
    },
  ],
};

