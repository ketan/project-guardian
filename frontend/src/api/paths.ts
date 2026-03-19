import {
  CONFIG_SECTION_KEYS,
  CONFIG_SECTION_PATH_SEGMENTS,
  PUBLISHER_SLOT_KEYS,
  type ConfigSectionKey,
  type PublisherSlotKey,
} from "./sectionKeys";

export const STATUS_PATH = "/api/v1/status";
export const OTA_UPLOAD_PATH = "/api/v1/admin/ota";

export const CONFIG_SECTION_PATHS: Record<ConfigSectionKey, { read: string; update: string }> =
  Object.fromEntries(
    CONFIG_SECTION_KEYS.map((key) => {
      const path = `/api/v1/config/${CONFIG_SECTION_PATH_SEGMENTS[key]}`;
      return [key, { read: path, update: path }];
    }),
  ) as Record<ConfigSectionKey, { read: string; update: string }>;

export const PUBLISHER_SLOT_PATHS: Record<PublisherSlotKey, { read: string; update: string }> =
  Object.fromEntries(
    PUBLISHER_SLOT_KEYS.map((slot) => {
      const path = `/api/v1/config/publishers/${slot}`;
      return [slot, { read: path, update: path }];
    }),
  ) as Record<PublisherSlotKey, { read: string; update: string }>;
