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
  WebUiConfig,
  WindyPublisherView,
  WundergroundPublisherView,
} from "./generated/schemas/index.zod";
import { CONFIG_SECTION_PATHS, PUBLISHER_SLOT_PATHS } from "./paths";
import { requestJson } from "./request";

const SCHEMA_VERSION = 1;
export type LoadableSectionKey = ConfigSectionKey | `publishers.${PublisherSlotKey}`;
export type RefreshedConfigMap = Partial<
  Record<ConfigSectionKey, unknown> &
    Record<`publishers.${PublisherSlotKey}`, unknown>
>;

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

async function yieldForRender() {
  await new Promise((resolve) => window.setTimeout(resolve, 0));
}

export async function fetchConfig(
  settings: ApiConnectionSettings,
  onSectionStart?: (section: LoadableSectionKey) => void,
  onSectionLoaded?: (section: LoadableSectionKey, value: unknown) => void,
): Promise<UiConfig> {
  onSectionStart?.("station");
  const station = (await readConfigSection("station", settings)) as z.infer<typeof StationConfig>;
  onSectionLoaded?.("station", station);
  await yieldForRender();
  onSectionStart?.("sampling");
  const sampling = (await readConfigSection("sampling", settings)) as z.infer<typeof SamplingConfig>;
  onSectionLoaded?.("sampling", sampling);
  await yieldForRender();
  onSectionStart?.("smoothing");
  const smoothing = (await readConfigSection("smoothing", settings)) as z.infer<typeof SmoothingConfig>;
  onSectionLoaded?.("smoothing", smoothing);
  await yieldForRender();
  onSectionStart?.("storage");
  const storage = (await readConfigSection("storage", settings)) as z.infer<typeof StorageConfig>;
  onSectionLoaded?.("storage", storage);
  await yieldForRender();
  onSectionStart?.("network");
  const network = (await readConfigSection("network", settings)) as z.infer<typeof NetworkConfig>;
  onSectionLoaded?.("network", network);
  await yieldForRender();
  onSectionStart?.("smsAdmin");
  const smsAdmin = (await readConfigSection("smsAdmin", settings)) as z.infer<typeof SmsAdminConfig>;
  onSectionLoaded?.("smsAdmin", smsAdmin);
  await yieldForRender();
  onSectionStart?.("webUi");
  const webUi = (await readConfigSection("webUi", settings)) as z.infer<typeof WebUiConfig>;
  onSectionLoaded?.("webUi", webUi);
  await yieldForRender();
  onSectionStart?.("sensors");
  const sensors = (await readConfigSection("sensors", settings)) as z.infer<typeof SensorConfig>[];
  onSectionLoaded?.("sensors", sensors);
  await yieldForRender();
  onSectionStart?.("publishers.wunderground");
  const wunderground = (await readPublisherSlot("wunderground", settings)) as z.infer<
    typeof WundergroundPublisherView
  >;
  onSectionLoaded?.("publishers.wunderground", wunderground);
  await yieldForRender();
  onSectionStart?.("publishers.windy");
  const windy = (await readPublisherSlot("windy", settings)) as z.infer<typeof WindyPublisherView>;
  onSectionLoaded?.("publishers.windy", windy);
  await yieldForRender();
  onSectionStart?.("publishers.mqtt");
  const mqtt = (await readPublisherSlot("mqtt", settings)) as z.infer<typeof MqttPublisherView>;
  onSectionLoaded?.("publishers.mqtt", mqtt);
  await yieldForRender();

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
): Promise<RefreshedConfigMap> {
  const refreshed: RefreshedConfigMap = {};

  for (const section of sections) {
    onSectionStart?.(section);
    refreshed[section] = await saveConfigSection(section, config, settings);
  }

  for (const slot of getDirtyPublisherSlots(dirtyFields)) {
    onSectionStart?.(`publishers.${slot}`);
    refreshed[`publishers.${slot}`] = await savePublisherSlot(slot, config, settings);
  }

  return refreshed;
}
