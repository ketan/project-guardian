export const CONFIG_SECTION_KEYS = [
  "station",
  "sampling",
  "smoothing",
  "storage",
  "network",
  "smsAdmin",
  "webUi",
  "sensors",
] as const;

export type ConfigSectionKey = (typeof CONFIG_SECTION_KEYS)[number];

export const CONFIG_SECTION_PATH_SEGMENTS: Record<ConfigSectionKey, string> = {
  station: "station",
  sampling: "sampling",
  smoothing: "smoothing",
  storage: "storage",
  network: "network",
  smsAdmin: "sms-admin",
  webUi: "web-ui",
  sensors: "sensors",
};

export const PUBLISHER_SLOT_KEYS = ["wunderground", "windy", "mqtt"] as const;

export type PublisherSlotKey = (typeof PUBLISHER_SLOT_KEYS)[number];

export function getPublisherLoadableKey(slot: PublisherSlotKey) {
  return `publishers.${slot}` as const;
}
