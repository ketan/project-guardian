import type { FieldNamesMarkedBoolean } from "react-hook-form";
import { z } from "zod";
import { ConfigSectionSchemas, type ConfigSectionKey, UiConfigSchema, type UiConfig } from "./contracts";
import type { ApiConnectionSettings } from "./runtime";
import {
  DeviceConfigView,
  NetworkConfig,
  PublisherConfigView,
  SamplingConfig,
  SensorConfig,
  SmsAdminConfig,
  SmoothingConfig,
  StationConfig,
  StorageConfig,
  WebUiConfig,
} from "./generated/schemas/index.zod";
import {
  getGetConfigUrl,
  getUpdateNetworkConfigUrl,
  getUpdatePublishersConfigUrl,
  getUpdateSamplingConfigUrl,
  getUpdateSensorsConfigUrl,
  getUpdateSmsAdminConfigUrl,
  getUpdateSmoothingConfigUrl,
  getUpdateStationConfigUrl,
  getUpdateStorageConfigUrl,
  getUpdateWebUiConfigUrl,
} from "./generated/client";

type ConfigSectionRequest = {
  getUrl: () => string;
  responseSchema: z.ZodTypeAny;
};

const configSectionRequests: Record<ConfigSectionKey, ConfigSectionRequest> = {
  station: {
    getUrl: getUpdateStationConfigUrl,
    responseSchema: StationConfig,
  },
  sampling: {
    getUrl: getUpdateSamplingConfigUrl,
    responseSchema: SamplingConfig,
  },
  smoothing: {
    getUrl: getUpdateSmoothingConfigUrl,
    responseSchema: SmoothingConfig,
  },
  storage: {
    getUrl: getUpdateStorageConfigUrl,
    responseSchema: StorageConfig,
  },
  network: {
    getUrl: getUpdateNetworkConfigUrl,
    responseSchema: NetworkConfig,
  },
  smsAdmin: {
    getUrl: getUpdateSmsAdminConfigUrl,
    responseSchema: SmsAdminConfig,
  },
  webUi: {
    getUrl: getUpdateWebUiConfigUrl,
    responseSchema: WebUiConfig,
  },
  sensors: {
    getUrl: getUpdateSensorsConfigUrl,
    responseSchema: z.array(SensorConfig),
  },
  publishers: {
    getUrl: getUpdatePublishersConfigUrl,
    responseSchema: z.array(PublisherConfigView),
  },
};

function hasDirtyValue(value: unknown): boolean {
  if (value === true) {
    return true;
  }

  if (Array.isArray(value)) {
    return value.some(hasDirtyValue);
  }

  if (typeof value === "object" && value !== null) {
    return Object.values(value).some(hasDirtyValue);
  }

  return false;
}

function resolveApiUrl(path: string, settings: ApiConnectionSettings): string {
  const baseUrl = settings.baseUrl.trim();

  if (!baseUrl) {
    return path;
  }

  return new URL(path, baseUrl).toString();
}

function buildHeaders(settings: ApiConnectionSettings, hasBody: boolean) {
  const headers = new Headers();

  if (hasBody) {
    headers.set("Content-Type", "application/json");
  }

  if (settings.apiKey.trim()) {
    headers.set("Authorization", `Bearer ${settings.apiKey.trim()}`);
  }

  return headers;
}

async function requestJson<T>(
  path: string,
  settings: ApiConnectionSettings,
  init: RequestInit,
  schema: z.ZodType<T>,
): Promise<T> {
  const response = await fetch(resolveApiUrl(path, settings), {
    ...init,
    headers: buildHeaders(settings, init.body !== undefined),
  });

  const body = [204, 205, 304].includes(response.status) ? null : await response.text();
  const parsedBody = body ? (JSON.parse(body) as unknown) : {};

  if (!response.ok) {
    const message =
      typeof parsedBody === "object" &&
      parsedBody !== null &&
      "message" in parsedBody &&
      typeof parsedBody.message === "string"
        ? parsedBody.message
        : `${response.status} ${response.statusText}`.trim();

    throw new Error(message);
  }

  return schema.parse(parsedBody);
}

export function getDirtySections(
  dirtyFields: FieldNamesMarkedBoolean<UiConfig>,
): ConfigSectionKey[] {
  const topLevel = dirtyFields as Partial<Record<ConfigSectionKey, unknown>>;

  return (Object.keys(configSectionRequests) as ConfigSectionKey[]).filter((section) =>
    hasDirtyValue(topLevel[section]),
  );
}

export async function fetchConfig(settings: ApiConnectionSettings): Promise<UiConfig> {
  const config = await requestJson(getGetConfigUrl(), settings, { method: "GET" }, DeviceConfigView);
  return UiConfigSchema.parse(config);
}

export async function saveConfigSection(
  section: ConfigSectionKey,
  config: UiConfig,
  settings: ApiConnectionSettings,
): Promise<unknown> {
  const payload = ConfigSectionSchemas[section].parse(config[section]);
  const request = configSectionRequests[section];

  return requestJson(
    request.getUrl(),
    settings,
    {
      method: "PUT",
      body: JSON.stringify(payload),
    },
    request.responseSchema,
  );
}

export async function saveConfigSectionsSequentially(
  sections: ConfigSectionKey[],
  config: UiConfig,
  settings: ApiConnectionSettings,
  onSectionStart?: (section: ConfigSectionKey) => void,
): Promise<void> {
  for (const section of sections) {
    onSectionStart?.(section);
    await saveConfigSection(section, config, settings);
  }
}
