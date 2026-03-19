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
  const parsedBody = body ? (JSON.parse(body) as unknown) : {};

  if (!response.ok) {
    const message =
      typeof parsedBody === "object" &&
      parsedBody !== null &&
      "message" in parsedBody &&
      typeof parsedBody.message === "string"
        ? parsedBody.message
        : `${response.status} ${response.statusText}`.trim();

    throw new Error(message);
  }

  return schema.parse(parsedBody);
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
  const parsedBody = body ? (JSON.parse(body) as unknown) : {};

  if (!response.ok) {
    const message =
      typeof parsedBody === "object" &&
      parsedBody !== null &&
      "message" in parsedBody &&
      typeof parsedBody.message === "string"
        ? parsedBody.message
        : `${response.status} ${response.statusText}`.trim();

    throw new Error(message);
  }

  return schema.parse(parsedBody);
}
