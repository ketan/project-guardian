import { useEffect, useState } from "react";
import {
  getApiConnectionSettings,
  normalizeApiBaseUrl,
  setApiConnectionSettings,
  type ApiConnectionSettings,
} from "../api/runtime";

export function useApiConnectionSettings() {
  const [settings, setSettings] = useState<ApiConnectionSettings>(() => getApiConnectionSettings());

  useEffect(() => {
    setApiConnectionSettings(settings);
  }, [settings]);

  const updateBaseUrl = (baseUrl: string) => {
    setSettings((current) => ({
      ...current,
      baseUrl,
    }));
  };

  const updateApiKey = (apiKey: string) => {
    setSettings((current) => ({
      ...current,
      apiKey,
    }));
  };

  return {
    settings,
    updateBaseUrl,
    updateApiKey,
    normalizedBaseUrl: normalizeApiBaseUrl(settings.baseUrl),
  };
}
