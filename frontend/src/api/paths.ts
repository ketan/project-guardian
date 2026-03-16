import type { ConfigSectionKey, PublisherSlotKey } from "./contracts";

export const CONFIG_SECTION_PATHS: Record<
  ConfigSectionKey,
  { read: string; update: string }
> = {
  station: {
    read: "/api/v1/config/station",
    update: "/api/v1/config/station",
  },
  sampling: {
    read: "/api/v1/config/sampling",
    update: "/api/v1/config/sampling",
  },
  smoothing: {
    read: "/api/v1/config/smoothing",
    update: "/api/v1/config/smoothing",
  },
  storage: {
    read: "/api/v1/config/storage",
    update: "/api/v1/config/storage",
  },
  network: {
    read: "/api/v1/config/network",
    update: "/api/v1/config/network",
  },
  smsAdmin: {
    read: "/api/v1/config/sms-admin",
    update: "/api/v1/config/sms-admin",
  },
  webUi: {
    read: "/api/v1/config/web-ui",
    update: "/api/v1/config/web-ui",
  },
  sensors: {
    read: "/api/v1/config/sensors",
    update: "/api/v1/config/sensors",
  },
};

export const PUBLISHER_SLOT_PATHS: Record<
  PublisherSlotKey,
  { read: string; update: string }
> = {
  wunderground: {
    read: "/api/v1/config/publishers/wunderground",
    update: "/api/v1/config/publishers/wunderground",
  },
  windy: {
    read: "/api/v1/config/publishers/windy",
    update: "/api/v1/config/publishers/windy",
  },
  webhook: {
    read: "/api/v1/config/publishers/webhook",
    update: "/api/v1/config/publishers/webhook",
  },
  mqtt: {
    read: "/api/v1/config/publishers/mqtt",
    update: "/api/v1/config/publishers/mqtt",
  },
};
