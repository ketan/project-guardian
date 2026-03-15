import { useCallback, useEffect, useState } from "react";
import type { FieldNamesMarkedBoolean } from "react-hook-form";
import type { ConfigSectionKey, UiConfig } from "../api/contracts";
import { hasApiConnectionSettings, type ApiConnectionSettings } from "../api/runtime";
import { fetchConfig, getDirtySections, saveConfigSectionsSequentially } from "../api/configApi";

export function useDeviceConfig(connection: ApiConnectionSettings) {
  const [config, setConfig] = useState<UiConfig | null>(null);
  const [isLoading, setIsLoading] = useState(false);
  const [loadError, setLoadError] = useState<string | null>(null);
  const canConnect = hasApiConnectionSettings(connection);

  const reload = useCallback(async () => {
    if (!canConnect) {
      setConfig(null);
      setLoadError(null);
      setIsLoading(false);
      return;
    }

    setIsLoading(true);
    setLoadError(null);

    try {
      const nextConfig = await fetchConfig(connection);
      setConfig(nextConfig);
    } catch (error) {
      setLoadError(error instanceof Error ? error.message : "Failed to load configuration");
    } finally {
      setIsLoading(false);
    }
  }, [canConnect, connection]);

  useEffect(() => {
    void reload();
  }, [reload]);

  return { config, isLoading, loadError, reload, canConnect };
}

export function useSequentialConfigSave(connection: ApiConnectionSettings) {
  const [isSaving, setIsSaving] = useState(false);
  const [saveError, setSaveError] = useState<string | null>(null);
  const [savingSection, setSavingSection] = useState<ConfigSectionKey | null>(null);
  const [lastSavedAt, setLastSavedAt] = useState<Date | null>(null);
  const canConnect = hasApiConnectionSettings(connection);

  const save = useCallback(
    async (config: UiConfig, dirtyFields: FieldNamesMarkedBoolean<UiConfig>) => {
      if (!canConnect) {
        setSaveError("Enter both the backend API base URL and API key before saving.");
        return { savedSections: [] as ConfigSectionKey[] };
      }

      const dirtySections = getDirtySections(dirtyFields);

      if (dirtySections.length === 0) {
        return { savedSections: [] as ConfigSectionKey[] };
      }

      setIsSaving(true);
      setSaveError(null);

      try {
        await saveConfigSectionsSequentially(dirtySections, config, connection, setSavingSection);
        setLastSavedAt(new Date());
        return { savedSections: dirtySections };
      } catch (error) {
        setSaveError(error instanceof Error ? error.message : "Failed to save configuration");
        throw error;
      } finally {
        setSavingSection(null);
        setIsSaving(false);
      }
    },
    [canConnect, connection],
  );

  return { save, isSaving, saveError, savingSection, lastSavedAt, canConnect };
}
