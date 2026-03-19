import {useCallback, useEffect, useState} from "react";
import type {FieldNamesMarkedBoolean} from "react-hook-form";
import type {UiConfig} from "../api/contracts";
import {hasApiConnectionSettings, type ApiConnectionSettings} from "../api/runtime";
import {mergeSectionValue, type LoadableSectionKey, type RefreshedConfigMap} from "../api/configSections";
import {
  fetchConfig,
  getDirtySections,
  saveConfigSectionsSequentially,
} from "../api/configApi";
import type {ConfigSectionKey, PublisherSlotKey} from "../api/sectionKeys";
type PartialUiConfig = Partial<Omit<UiConfig, "publishers">> & {
  publishers?: Partial<UiConfig["publishers"]>;
};

export function useDeviceConfig(connection: ApiConnectionSettings) {
  const [config, setConfig] = useState<PartialUiConfig | null>(null);
  const [isLoading, setIsLoading] = useState(false);
  const [loadingSection, setLoadingSection] = useState<
    ConfigSectionKey | `publishers.${PublisherSlotKey}` | null
  >(null);
  const [loadedSections, setLoadedSections] = useState<Set<LoadableSectionKey>>(new Set());
  const [loadError, setLoadError] = useState<string | null>(null);
  const canConnect = hasApiConnectionSettings(connection);

  const reload = useCallback(async () => {
    if (!canConnect) {
      setConfig(null);
      setLoadError(null);
      setIsLoading(false);
      setLoadingSection(null);
      setLoadedSections(new Set());
      return;
    }

    setIsLoading(true);
    setLoadError(null);
    setConfig(null);
    setLoadedSections(new Set());

    try {
      const nextConfig = await fetchConfig(connection, setLoadingSection, (section, value) => {
        setLoadedSections((current) => new Set(current).add(section));
        setConfig((current) => mergeSectionValue(current ?? {}, section, value));
      });
      setConfig(nextConfig);
    } catch (error) {
      setLoadError(error instanceof Error ? error.message : "Failed to load configuration");
    } finally {
      setLoadingSection(null);
      setIsLoading(false);
    }
  }, [canConnect, connection]);

  useEffect(() => {
    void reload();
  }, [reload]);

  return {config, isLoading, loadingSection, loadedSections, loadError, reload, canConnect};
}

export function useSequentialConfigSave(connection: ApiConnectionSettings) {
  const [isSaving, setIsSaving] = useState(false);
  const [saveError, setSaveError] = useState<string | null>(null);
  const [savingSection, setSavingSection] = useState<
    ConfigSectionKey | `publishers.${PublisherSlotKey}` | null
  >(null);
  const [lastSavedAt, setLastSavedAt] = useState<Date | null>(null);
  const canConnect = hasApiConnectionSettings(connection);

  const save = useCallback(
    async (config: UiConfig, dirtyFields: FieldNamesMarkedBoolean<UiConfig>) => {
      if (!canConnect) {
        setSaveError("Enter both the backend API base URL and API key before saving.");
        return {savedSections: [] as ConfigSectionKey[], refreshedSections: {} as RefreshedConfigMap};
      }

      const dirtySections = getDirtySections(dirtyFields);

      if (dirtySections.length === 0) {
        return {savedSections: [] as ConfigSectionKey[], refreshedSections: {} as RefreshedConfigMap};
      }

      setIsSaving(true);
      setSaveError(null);

      try {
        const refreshedSections = await saveConfigSectionsSequentially(
          dirtySections,
          config,
          connection,
          dirtyFields,
          setSavingSection,
        );
        setLastSavedAt(new Date());
        return {savedSections: dirtySections, refreshedSections};
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

  return {save, isSaving, saveError, savingSection, lastSavedAt, canConnect};
}
