import type { FieldNamesMarkedBoolean } from "react-hook-form";
import { z } from "zod";
import {
  ConfigSectionSchemas,
  PublisherSectionSchemas,
  UiConfigSchema,
  type UiConfig,
} from "./contracts";
import type { ApiConnectionSettings } from "./runtime";
import type { LoadableSectionKey, RefreshedConfigMap } from "./configSections";
import {
  CONFIG_SECTION_KEYS,
  PUBLISHER_SLOT_KEYS,
  getPublisherLoadableKey,
  type ConfigSectionKey,
  type PublisherSlotKey,
} from "./sectionKeys";
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

  return CONFIG_SECTION_KEYS.filter((section) =>
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

  return PUBLISHER_SLOT_KEYS.filter((slot) =>
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

const orderedConfigLoaders = [
  ...CONFIG_SECTION_KEYS.map((key) => ({ key, load: readConfigSection })),
  ...PUBLISHER_SLOT_KEYS.map((slot) => ({
    key: getPublisherLoadableKey(slot),
    load: readPublisherSlot,
    slot,
  })),
] as const;

export async function fetchConfig(
  settings: ApiConnectionSettings,
  onSectionStart?: (section: LoadableSectionKey) => void,
  onSectionLoaded?: (section: LoadableSectionKey, value: unknown) => void,
): Promise<UiConfig> {
  const loadedSections: Partial<Record<LoadableSectionKey, unknown>> = {};

  for (const loader of orderedConfigLoaders) {
    onSectionStart?.(loader.key);
    const value =
      "slot" in loader
        ? await loader.load(loader.slot, settings)
        : await loader.load(loader.key, settings);
    loadedSections[loader.key] = value;
    onSectionLoaded?.(loader.key, value);
    await yieldForRender();
  }

  return UiConfigSchema.parse({
    schemaVersion: SCHEMA_VERSION,
    station: loadedSections.station as z.infer<typeof StationConfig>,
    sampling: loadedSections.sampling as z.infer<typeof SamplingConfig>,
    smoothing: loadedSections.smoothing as z.infer<typeof SmoothingConfig>,
    storage: loadedSections.storage as z.infer<typeof StorageConfig>,
    network: loadedSections.network as z.infer<typeof NetworkConfig>,
    smsAdmin: loadedSections.smsAdmin as z.infer<typeof SmsAdminConfig>,
    webUi: loadedSections.webUi as z.infer<typeof WebUiConfig>,
    sensors: loadedSections.sensors as z.infer<typeof SensorConfig>[],
    publishers: {
      wunderground: loadedSections["publishers.wunderground"] as z.infer<typeof WundergroundPublisherView>,
      windy: loadedSections["publishers.windy"] as z.infer<typeof WindyPublisherView>,
      mqtt: loadedSections["publishers.mqtt"] as z.infer<typeof MqttPublisherView>,
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
