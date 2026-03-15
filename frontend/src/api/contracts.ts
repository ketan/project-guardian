import { z } from "zod";
import {
  DeviceConfigView,
  DeviceStatusResponse,
  UpdateNetworkConfigBody,
  UpdatePublishersConfigBody,
  UpdateSamplingConfigBody,
  UpdateSensorsConfigBody,
  UpdateSmsAdminConfigBody,
  UpdateSmoothingConfigBody,
  UpdateStationConfigBody,
  UpdateStorageConfigBody,
  UpdateWebUiConfigBody,
} from "./generated/schemas/index.zod";

const UiPublisherSchema = z.discriminatedUnion("type", [
  z.object({
    id: z.string(),
    type: z.literal("wunderground"),
    enabled: z.boolean(),
    publishIntervalSeconds: z.number().min(30),
    includeHistoryWindowMinutes: z.number().min(1).optional(),
    stationId: z.string(),
    apiKeyConfigured: z.boolean().optional(),
    apiKey: z.string().optional(),
  }),
  z.object({
    id: z.string(),
    type: z.literal("windy"),
    enabled: z.boolean(),
    publishIntervalSeconds: z.number().min(30),
    includeHistoryWindowMinutes: z.number().min(1).optional(),
    stationId: z.string(),
    apiKeyConfigured: z.boolean().optional(),
    apiKey: z.string().optional(),
  }),
  z.object({
    id: z.string(),
    type: z.literal("webhook"),
    enabled: z.boolean(),
    publishIntervalSeconds: z.number().min(30),
    includeHistoryWindowMinutes: z.number().min(1).optional(),
    endpoint: z.url(),
    authHeaderConfigured: z.boolean().optional(),
    authHeader: z.string().optional(),
  }),
  z.object({
    id: z.string(),
    type: z.literal("mqtt"),
    enabled: z.boolean(),
    publishIntervalSeconds: z.number().min(30),
    includeHistoryWindowMinutes: z.number().min(1).optional(),
    brokerUrl: z.url(),
    topic: z.string(),
    username: z.string(),
    passwordConfigured: z.boolean().optional(),
    password: z.string().optional(),
  }),
]);

export const DeviceStatusSchema = DeviceStatusResponse;

export const UiConfigSchema = DeviceConfigView.extend({
  publishers: z.array(UiPublisherSchema),
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
  publishers: UpdatePublishersConfigBody,
} as const;

export type DeviceStatus = z.infer<typeof DeviceStatusSchema>;
export type UiConfig = z.infer<typeof UiConfigSchema>;
export type UiPublisher = z.infer<typeof UiPublisherSchema>;
export type ConfigSectionKey = keyof typeof ConfigSectionSchemas;
