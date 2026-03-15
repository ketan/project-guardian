import type { FieldNamesMarkedBoolean } from "react-hook-form";
import { ConfigSectionSchemas, type ConfigSectionKey, UiConfigSchema, type UiConfig } from "./contracts";
import {
  getConfig,
  updateNetworkConfig,
  updatePublishersConfig,
  updateSamplingConfig,
  updateSensorsConfig,
  updateSmsAdminConfig,
  updateSmoothingConfig,
  updateStationConfig,
  updateStorageConfig,
  updateWebUiConfig,
} from "./generated/client";

const configSectionRequests: Record<ConfigSectionKey, (payload: any) => Promise<unknown>> = {
  station: updateStationConfig,
  sampling: updateSamplingConfig,
  smoothing: updateSmoothingConfig,
  storage: updateStorageConfig,
  network: updateNetworkConfig,
  smsAdmin: updateSmsAdminConfig,
  webUi: updateWebUiConfig,
  sensors: updateSensorsConfig,
  publishers: updatePublishersConfig,
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

export async function fetchConfig(): Promise<UiConfig> {
  const config = await getConfig();
  return UiConfigSchema.parse(config);
}

export async function saveConfigSection(
  section: ConfigSectionKey,
  config: UiConfig,
): Promise<unknown> {
  const payload = ConfigSectionSchemas[section].parse(config[section]);

  return configSectionRequests[section](payload);
}

export async function saveConfigSectionsSequentially(
  sections: ConfigSectionKey[],
  config: UiConfig,
  onSectionStart?: (section: ConfigSectionKey) => void,
): Promise<void> {
  for (const section of sections) {
    onSectionStart?.(section);
    await saveConfigSection(section, config);
  }
}
