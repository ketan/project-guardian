import { describe, expect, it, vi } from "vitest";
import type { FieldNamesMarkedBoolean } from "react-hook-form";
import { fetchConfig, saveConfigSectionsSequentially } from "./configApi";
import type { UiConfig } from "./contracts";
import { testConfig } from "../test/fixtures";

type Deferred<T> = {
  promise: Promise<T>;
  resolve: (value: T) => void;
};

function deferred<T>(): Deferred<T> {
  let resolve!: (value: T) => void;
  const promise = new Promise<T>((nextResolve) => {
    resolve = nextResolve;
  });

  return { promise, resolve };
}

function jsonResponse(body: unknown) {
  return new Response(JSON.stringify(body), {
    status: 200,
    headers: { "Content-Type": "application/json" },
  });
}

describe("configApi", () => {
  it("fetches config sections in sequence", async () => {
    const stationResponse = deferred<Response>();
    const samplingResponse = deferred<Response>();
    const calls: string[] = [];

    vi.spyOn(globalThis, "fetch").mockImplementation((input) => {
      const url = typeof input === "string" ? input : input.toString();
      const path = new URL(url, "http://device.local").pathname;
      calls.push(path);

      switch (path) {
        case "/api/v1/config/station":
          return stationResponse.promise;
        case "/api/v1/config/sampling":
          return samplingResponse.promise;
        case "/api/v1/config/smoothing":
          return Promise.resolve(jsonResponse(testConfig.smoothing));
        case "/api/v1/config/storage":
          return Promise.resolve(jsonResponse(testConfig.storage));
        case "/api/v1/config/network":
          return Promise.resolve(jsonResponse(testConfig.network));
        case "/api/v1/config/sms-admin":
          return Promise.resolve(jsonResponse(testConfig.smsAdmin));
        case "/api/v1/config/web-ui":
          return Promise.resolve(jsonResponse(testConfig.webUi));
        case "/api/v1/config/sensors":
          return Promise.resolve(jsonResponse(testConfig.sensors));
        case "/api/v1/config/publishers/wunderground":
          return Promise.resolve(jsonResponse(testConfig.publishers.wunderground));
        case "/api/v1/config/publishers/windy":
          return Promise.resolve(jsonResponse(testConfig.publishers.windy));
        case "/api/v1/config/publishers/mqtt":
          return Promise.resolve(jsonResponse(testConfig.publishers.mqtt));
        default:
          throw new Error(`Unexpected fetch ${path}`);
      }
    });

    const promise = fetchConfig({ baseUrl: "http://device.local", apiKey: "secret" });

    expect(calls).toEqual(["/api/v1/config/station"]);

    stationResponse.resolve(jsonResponse(testConfig.station));
    await vi.waitFor(() => {
      expect(calls).toEqual([
        "/api/v1/config/station",
        "/api/v1/config/sampling",
      ]);
    });

    samplingResponse.resolve(jsonResponse(testConfig.sampling));
    await promise;
  });

  it("saves only dirty sections and does so in sequence", async () => {
    const stationPut = deferred<Response>();
    const mqttPut = deferred<Response>();
    const calls: string[] = [];

    vi.spyOn(globalThis, "fetch").mockImplementation((input, init) => {
      const url = typeof input === "string" ? input : input.toString();
      const path = new URL(url, "http://device.local").pathname;
      const method = (init?.method ?? "GET").toUpperCase();
      calls.push(`${method} ${path}`);

      if (method !== "PUT") {
        throw new Error(`Unexpected non-PUT request ${method} ${path}`);
      }

      switch (path) {
        case "/api/v1/config/station":
          return stationPut.promise;
        case "/api/v1/config/publishers/mqtt":
          return mqttPut.promise;
        default:
          throw new Error(`Unexpected PUT ${path}`);
      }
    });

    const dirtyFields = {
      station: { stationName: true },
      publishers: { mqtt: { topic: true } },
    } as FieldNamesMarkedBoolean<UiConfig>;

    const promise = saveConfigSectionsSequentially(
      ["station"],
      testConfig,
      { baseUrl: "http://device.local", apiKey: "secret" },
      dirtyFields,
    );

    expect(calls).toEqual(["PUT /api/v1/config/station"]);

    stationPut.resolve(jsonResponse(testConfig.station));
    await vi.waitFor(() => {
      expect(calls).toEqual([
        "PUT /api/v1/config/station",
        "PUT /api/v1/config/publishers/mqtt",
      ]);
    });

    mqttPut.resolve(jsonResponse(testConfig.publishers.mqtt));
    await promise;
  });
});
