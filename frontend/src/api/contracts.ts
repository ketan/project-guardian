import { z } from "zod";
import {
  DeviceConfigView,
  DeviceStatusResponse,
  MqttPublisherView,
  UpdateNetworkConfigBody,
  UpdateMqttPublisherConfigBody,
  UpdateSamplingConfigBody,
  UpdateSensorsConfigBody,
  UpdateSmsAdminConfigBody,
  UpdateSmoothingConfigBody,
  UpdateStationConfigBody,
  UpdateStorageConfigBody,
  UpdateWebhookPublisherConfigBody,
  UpdateWebUiConfigBody,
  UpdateWindyPublisherConfigBody,
  UpdateWundergroundPublisherConfigBody,
  WebhookPublisherView,
  WindyPublisherView,
  WundergroundPublisherView,
} from "./generated/schemas/index.zod";

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

const UiWebhookPublisherSchema = WebhookPublisherView.and(
  z.object({
    authHeader: z.string().optional(),
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
  webhook: UiWebhookPublisherSchema.optional(),
  mqtt: UiMqttPublisherSchema.optional(),
});

export const DeviceStatusSchema = DeviceStatusResponse;

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
} as const;

export const PublisherSectionSchemas = {
  wunderground: UpdateWundergroundPublisherConfigBody,
  windy: UpdateWindyPublisherConfigBody,
  webhook: UpdateWebhookPublisherConfigBody,
  mqtt: UpdateMqttPublisherConfigBody,
} as const;

export type DeviceStatus = z.infer<typeof DeviceStatusSchema>;
export type UiConfig = z.infer<typeof UiConfigSchema>;
export type ConfigSectionKey = keyof typeof ConfigSectionSchemas;
export type PublisherSlotKey = keyof typeof PublisherSectionSchemas;
export type UiPublishers = z.infer<typeof UiPublishersSchema>;
export type UiWundergroundPublisher = z.infer<typeof UiWundergroundPublisherSchema>;
export type UiWindyPublisher = z.infer<typeof UiWindyPublisherSchema>;
export type UiWebhookPublisher = z.infer<typeof UiWebhookPublisherSchema>;
export type UiMqttPublisher = z.infer<typeof UiMqttPublisherSchema>;
