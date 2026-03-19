import {useCallback, useEffect, useState} from "react";
import type {FieldNamesMarkedBoolean} from "react-hook-form";
import type {ConfigSectionKey, PublisherSlotKey, UiConfig} from "../api/contracts";
import {hasApiConnectionSettings, type ApiConnectionSettings} from "../api/runtime";
import {
  fetchConfig,
  getDirtySections,
  saveConfigSectionsSequentially,
  type LoadableSectionKey,
} from "../api/configApi";

type PartialUiConfig = Partial<Omit<UiConfig, "publishers">> & {
  publishers?: Partial<UiConfig["publishers"]>;
};

function mergeLoadedSection(
  current: PartialUiConfig,
  section: LoadableSectionKey,
  value: unknown,
): PartialUiConfig {
  if (section.startsWith("publishers.")) {
    const slot = section.replace("publishers.", "") as PublisherSlotKey;

    return {
      ...current,
      publishers: {
        ...current.publishers,
        [slot]: value,
      },
    };
  }

  return {
    ...current,
    [section]: value,
  };
}

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
        setConfig((current) => mergeLoadedSection(current ?? {}, section, value));
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
        return {savedSections: [] as ConfigSectionKey[]};
      }

      const dirtySections = getDirtySections(dirtyFields);

      if (dirtySections.length === 0) {
        return {savedSections: [] as ConfigSectionKey[]};
      }

      setIsSaving(true);
      setSaveError(null);

      try {
        await saveConfigSectionsSequentially(
          dirtySections,
          config,
          connection,
          dirtyFields,
          setSavingSection,
        );
        setLastSavedAt(new Date());
        return {savedSections: dirtySections};
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
