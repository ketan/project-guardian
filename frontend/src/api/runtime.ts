export type ApiConnectionSettings = {
  baseUrl: string;
  apiKey: string;
};

const STORAGE_KEY = "guardian.apiConnection";

export function normalizeApiBaseUrl(value: string): string {
  const trimmed = value.trim();

  if (!trimmed) {
    return "";
  }

  const withScheme = /^[a-z]+:\/\//i.test(trimmed) ? trimmed : `http://${trimmed}`;
  return withScheme.replace(/\/+$/, "");
}

export function getApiConnectionSettings(): ApiConnectionSettings {
  if (typeof window === "undefined") {
    return { baseUrl: "", apiKey: "" };
  }

  const raw = window.localStorage.getItem(STORAGE_KEY);
  if (!raw) {
    return { baseUrl: "", apiKey: "" };
  }

  try {
    const parsed = JSON.parse(raw) as Partial<ApiConnectionSettings>;
    return {
      baseUrl: typeof parsed.baseUrl === "string" ? parsed.baseUrl : "",
      apiKey: typeof parsed.apiKey === "string" ? parsed.apiKey : "",
    };
  } catch {
    return { baseUrl: "", apiKey: "" };
  }
}

export function setApiConnectionSettings(settings: ApiConnectionSettings) {
  if (typeof window === "undefined") {
    return;
  }

  window.localStorage.setItem(
    STORAGE_KEY,
    JSON.stringify({
      baseUrl: normalizeApiBaseUrl(settings.baseUrl),
      apiKey: settings.apiKey,
    }),
  );
}

export function hasApiConnectionSettings(settings: ApiConnectionSettings): boolean {
  return Boolean(normalizeApiBaseUrl(settings.baseUrl) && settings.apiKey.trim());
}
