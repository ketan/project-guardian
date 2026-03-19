import { z } from "zod";
import type { ApiConnectionSettings } from "./runtime";

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

function safeParseJson(body: string | null): { ok: true; value: unknown } | { ok: false } {
  if (!body) {
    return { ok: true, value: {} };
  }

  try {
    return { ok: true, value: JSON.parse(body) as unknown };
  } catch {
    return { ok: false };
  }
}

export async function requestFormData<T>(
  path: string,
  settings: ApiConnectionSettings,
  formData: FormData,
  schema: z.ZodType<T>,
): Promise<T> {
  const response = await fetch(resolveApiUrl(path, settings), {
    method: "POST",
    headers: buildHeaders(settings, false),
    body: formData,
  });

  const body = [204, 205, 304].includes(response.status) ? null : await response.text();
  const parsedBody = safeParseJson(body);

  if (!response.ok) {
    if (!parsedBody.ok) {
      throw new Error("Request failed. Server returned an invalid error response.");
    }

    const message =
      typeof parsedBody.value === "object" &&
      parsedBody.value !== null &&
      "message" in parsedBody.value &&
      typeof parsedBody.value.message === "string"
        ? parsedBody.value.message
        : `${response.status} ${response.statusText}`.trim();

    throw new Error(message);
  }

  if (!parsedBody.ok) {
    throw new Error("Server returned an invalid JSON response.");
  }

  return schema.parse(parsedBody.value);
}

export async function requestJson<T>(
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
  const parsedBody = safeParseJson(body);

  if (!response.ok) {
    if (!parsedBody.ok) {
      throw new Error("Request failed. Server returned an invalid error response.");
    }

    const message =
      typeof parsedBody.value === "object" &&
      parsedBody.value !== null &&
      "message" in parsedBody.value &&
      typeof parsedBody.value.message === "string"
        ? parsedBody.value.message
        : `${response.status} ${response.statusText}`.trim();

    throw new Error(message);
  }

  if (!parsedBody.ok) {
    throw new Error("Server returned an invalid JSON response.");
  }

  return schema.parse(parsedBody.value);
}
