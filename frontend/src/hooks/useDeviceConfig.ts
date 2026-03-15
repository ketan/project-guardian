import { useCallback, useEffect, useState } from "react";
import type { FieldNamesMarkedBoolean } from "react-hook-form";
import type { ConfigSectionKey, UiConfig } from "../api/contracts";
import { fetchConfig, getDirtySections, saveConfigSectionsSequentially } from "../api/configApi";

export function useDeviceConfig() {
  const [config, setConfig] = useState<UiConfig | null>(null);
  const [isLoading, setIsLoading] = useState(true);
  const [loadError, setLoadError] = useState<string | null>(null);

  const reload = useCallback(async () => {
    setIsLoading(true);
    setLoadError(null);

    try {
      const nextConfig = await fetchConfig();
      setConfig(nextConfig);
    } catch (error) {
      setLoadError(error instanceof Error ? error.message : "Failed to load configuration");
    } finally {
      setIsLoading(false);
    }
  }, []);

  useEffect(() => {
    void reload();
  }, [reload]);

  return { config, isLoading, loadError, reload };
}

export function useSequentialConfigSave() {
  const [isSaving, setIsSaving] = useState(false);
  const [saveError, setSaveError] = useState<string | null>(null);
  const [savingSection, setSavingSection] = useState<ConfigSectionKey | null>(null);
  const [lastSavedAt, setLastSavedAt] = useState<Date | null>(null);

  const save = useCallback(
    async (config: UiConfig, dirtyFields: FieldNamesMarkedBoolean<UiConfig>) => {
      const dirtySections = getDirtySections(dirtyFields);

      if (dirtySections.length === 0) {
        return { savedSections: [] as ConfigSectionKey[] };
      }

      setIsSaving(true);
      setSaveError(null);

      try {
        await saveConfigSectionsSequentially(dirtySections, config, setSavingSection);
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
    [],
  );

  return { save, isSaving, saveError, savingSection, lastSavedAt };
}
