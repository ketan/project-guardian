import type { FieldNamesMarkedBoolean } from "react-hook-form";
import { z } from "zod";
import {
  ConfigSectionSchemas,
  PublisherSectionSchemas,
  type ConfigSectionKey,
  type PublisherSlotKey,
  UiConfigSchema,
  type UiConfig,
} from "./contracts";
import type { ApiConnectionSettings } from "./runtime";
import {
  DeviceConfigView,
  MqttPublisherView,
  NetworkConfig,
  SamplingConfig,
  SensorConfig,
  SmsAdminConfig,
  SmoothingConfig,
  StationConfig,
  StorageConfig,
  WebhookPublisherView,
  WebUiConfig,
  WindyPublisherView,
  WundergroundPublisherView,
} from "./generated/schemas/index.zod";
import { CONFIG_SECTION_PATHS, PUBLISHER_SLOT_PATHS } from "./paths";

const SCHEMA_VERSION = 1;

type ConfigSectionRequest = {
  readUrl: () => string;
  updateUrl: () => string;
  responseSchema: z.ZodTypeAny;
};

const configSectionRequests: Record<ConfigSectionKey, ConfigSectionRequest> = {
  station: {
    readUrl: () => CONFIG_SECTION_PATHS.station.read,
    updateUrl: () => CONFIG_SECTION_PATHS.station.update,
    responseSchema: StationConfig,
  },
  sampling: {
    readUrl: () => CONFIG_SECTION_PATHS.sampling.read,
    updateUrl: () => CONFIG_SECTION_PATHS.sampling.update,
    responseSchema: SamplingConfig,
  },
  smoothing: {
    readUrl: () => CONFIG_SECTION_PATHS.smoothing.read,
    updateUrl: () => CONFIG_SECTION_PATHS.smoothing.update,
    responseSchema: SmoothingConfig,
  },
  storage: {
    readUrl: () => CONFIG_SECTION_PATHS.storage.read,
    updateUrl: () => CONFIG_SECTION_PATHS.storage.update,
    responseSchema: StorageConfig,
  },
  network: {
    readUrl: () => CONFIG_SECTION_PATHS.network.read,
    updateUrl: () => CONFIG_SECTION_PATHS.network.update,
    responseSchema: NetworkConfig,
  },
  smsAdmin: {
    readUrl: () => CONFIG_SECTION_PATHS.smsAdmin.read,
    updateUrl: () => CONFIG_SECTION_PATHS.smsAdmin.update,
    responseSchema: SmsAdminConfig,
  },
  webUi: {
    readUrl: () => CONFIG_SECTION_PATHS.webUi.read,
    updateUrl: () => CONFIG_SECTION_PATHS.webUi.update,
    responseSchema: WebUiConfig,
  },
  sensors: {
    readUrl: () => CONFIG_SECTION_PATHS.sensors.read,
    updateUrl: () => CONFIG_SECTION_PATHS.sensors.update,
    responseSchema: z.array(SensorConfig),
  },
};

type PublisherSectionRequest = {
  readUrl: () => string;
  updateUrl: () => string;
  responseSchema: z.ZodTypeAny;
};

const publisherSectionRequests: Record<PublisherSlotKey, PublisherSectionRequest> = {
  wunderground: {
    readUrl: () => PUBLISHER_SLOT_PATHS.wunderground.read,
    updateUrl: () => PUBLISHER_SLOT_PATHS.wunderground.update,
    responseSchema: WundergroundPublisherView,
  },
  windy: {
    readUrl: () => PUBLISHER_SLOT_PATHS.windy.read,
    updateUrl: () => PUBLISHER_SLOT_PATHS.windy.update,
    responseSchema: WindyPublisherView,
  },
  webhook: {
    readUrl: () => PUBLISHER_SLOT_PATHS.webhook.read,
    updateUrl: () => PUBLISHER_SLOT_PATHS.webhook.update,
    responseSchema: WebhookPublisherView,
  },
  mqtt: {
    readUrl: () => PUBLISHER_SLOT_PATHS.mqtt.read,
    updateUrl: () => PUBLISHER_SLOT_PATHS.mqtt.update,
    responseSchema: MqttPublisherView,
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

function getDirtyPublisherSlots(
  dirtyFields: FieldNamesMarkedBoolean<UiConfig>,
): PublisherSlotKey[] {
  const publishers = dirtyFields.publishers as Partial<Record<PublisherSlotKey, unknown>> | undefined;

  if (!publishers) {
    return [];
  }

  return (Object.keys(publisherSectionRequests) as PublisherSlotKey[]).filter((slot) =>
    hasDirtyValue(publishers[slot]),
  );
}

async function readConfigSection(
  section: ConfigSectionKey,
  settings: ApiConnectionSettings,
): Promise<unknown> {
  const request = configSectionRequests[section];

  return requestJson(request.readUrl(), settings, { method: "GET" }, request.responseSchema);
}

async function readPublisherSlot(
  slot: PublisherSlotKey,
  settings: ApiConnectionSettings,
): Promise<unknown> {
  const request = publisherSectionRequests[slot];

  return requestJson(request.readUrl(), settings, { method: "GET" }, request.responseSchema);
}

export async function fetchConfig(settings: ApiConnectionSettings): Promise<UiConfig> {
  const station = (await readConfigSection("station", settings)) as z.infer<typeof StationConfig>;
  const sampling = (await readConfigSection("sampling", settings)) as z.infer<typeof SamplingConfig>;
  const smoothing = (await readConfigSection("smoothing", settings)) as z.infer<typeof SmoothingConfig>;
  const storage = (await readConfigSection("storage", settings)) as z.infer<typeof StorageConfig>;
  const network = (await readConfigSection("network", settings)) as z.infer<typeof NetworkConfig>;
  const smsAdmin = (await readConfigSection("smsAdmin", settings)) as z.infer<typeof SmsAdminConfig>;
  const webUi = (await readConfigSection("webUi", settings)) as z.infer<typeof WebUiConfig>;
  const sensors = (await readConfigSection("sensors", settings)) as z.infer<typeof SensorConfig>[];
  const wunderground = (await readPublisherSlot("wunderground", settings)) as z.infer<
    typeof WundergroundPublisherView
  >;
  const windy = (await readPublisherSlot("windy", settings)) as z.infer<typeof WindyPublisherView>;
  const webhook = (await readPublisherSlot("webhook", settings)) as z.infer<typeof WebhookPublisherView>;
  const mqtt = (await readPublisherSlot("mqtt", settings)) as z.infer<typeof MqttPublisherView>;

  return UiConfigSchema.parse({
    schemaVersion: SCHEMA_VERSION,
    station,
    sampling,
    smoothing,
    storage,
    network,
    smsAdmin,
    webUi,
    sensors,
    publishers: {
      wunderground,
      windy,
      webhook,
      mqtt,
    },
  } satisfies z.input<typeof DeviceConfigView>);
}

export async function saveConfigSection(
  section: ConfigSectionKey,
  config: UiConfig,
  settings: ApiConnectionSettings,
): Promise<unknown> {
  const payload = ConfigSectionSchemas[section].parse(config[section]);
  const request = configSectionRequests[section];

  return requestJson(
    request.updateUrl(),
    settings,
    {
      method: "PUT",
      body: JSON.stringify(payload),
    },
    request.responseSchema,
  );
}

async function savePublisherSlot(
  slot: PublisherSlotKey,
  config: UiConfig,
  settings: ApiConnectionSettings,
): Promise<unknown> {
  const publisher = config.publishers[slot];

  if (!publisher) {
    return null;
  }

  const payload = PublisherSectionSchemas[slot].parse(publisher);
  const request = publisherSectionRequests[slot];

  return requestJson(
    request.updateUrl(),
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
  dirtyFields: FieldNamesMarkedBoolean<UiConfig>,
  onSectionStart?: (section: ConfigSectionKey | `publishers.${PublisherSlotKey}`) => void,
): Promise<void> {
  for (const section of sections) {
    onSectionStart?.(section);
    await saveConfigSection(section, config, settings);
  }

  for (const slot of getDirtyPublisherSlots(dirtyFields)) {
    onSectionStart?.(`publishers.${slot}`);
    await savePublisherSlot(slot, config, settings);
  }
}
