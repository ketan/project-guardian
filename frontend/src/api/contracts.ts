import { z } from "zod";
import {
  DeviceConfigView,
  DeviceStatusResponse,
  MqttPublisherView,
  OtaUploadResponse,
  UpdateNetworkConfigBody,
  UpdateMqttPublisherConfigBody,
  UpdateSamplingConfigBody,
  UpdateSensorsConfigBody,
  UpdateSmsAdminConfigBody,
  UpdateSmoothingConfigBody,
  UpdateStationConfigBody,
  UpdateStorageConfigBody,
  UpdateWebUiConfigBody,
  UpdateWindyPublisherConfigBody,
  UpdateWundergroundPublisherConfigBody,
  WindyPublisherView,
  WundergroundPublisherView,
} from "./generated/schemas/index.zod";
import type { ConfigSectionKey, PublisherSlotKey } from "./sectionKeys";

const UiWundergroundPublisherSchema = WundergroundPublisherView.and(
  z.object({
    apiKey: z.string().optional(),
  }),
);

const UiWindyPublisherSchema = WindyPublisherView.and(
  z.object({
    apiKey: z.string().optional(),
  }),
);

const UiMqttPublisherSchema = MqttPublisherView.and(
  z.object({
    password: z.string().optional(),
  }),
);

export const UiPublishersSchema = z.object({
  wunderground: UiWundergroundPublisherSchema.optional(),
  windy: UiWindyPublisherSchema.optional(),
  mqtt: UiMqttPublisherSchema.optional(),
});

export const DeviceStatusSchema = DeviceStatusResponse;
export const OtaUploadResponseSchema = OtaUploadResponse;

export const UiConfigSchema = DeviceConfigView.extend({
  publishers: UiPublishersSchema,
});

export const ConfigSectionSchemas = {
  station: UpdateStationConfigBody,
  sampling: UpdateSamplingConfigBody,
  smoothing: UpdateSmoothingConfigBody,
  storage: UpdateStorageConfigBody,
  network: UpdateNetworkConfigBody,
  smsAdmin: UpdateSmsAdminConfigBody,
  webUi: UpdateWebUiConfigBody,
  sensors: UpdateSensorsConfigBody,
} as const satisfies Record<ConfigSectionKey, z.ZodTypeAny>;

export const PublisherSectionSchemas = {
  wunderground: UpdateWundergroundPublisherConfigBody,
  windy: UpdateWindyPublisherConfigBody,
  mqtt: UpdateMqttPublisherConfigBody,
} as const satisfies Record<PublisherSlotKey, z.ZodTypeAny>;

export type DeviceStatus = z.infer<typeof DeviceStatusSchema>;
export type OtaUploadResult = z.infer<typeof OtaUploadResponseSchema>;
export type UiConfig = z.infer<typeof UiConfigSchema>;
export type UiPublishers = z.infer<typeof UiPublishersSchema>;
export type UiWundergroundPublisher = z.infer<typeof UiWundergroundPublisherSchema>;
export type UiWindyPublisher = z.infer<typeof UiWindyPublisherSchema>;
export type UiMqttPublisher = z.infer<typeof UiMqttPublisherSchema>;
