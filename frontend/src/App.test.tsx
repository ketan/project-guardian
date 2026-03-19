import { fireEvent, render, screen, waitFor } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { MantineProvider } from "@mantine/core";
import { beforeEach, describe, expect, it, vi } from "vitest";
import type { UiConfig } from "./api/contracts";
import App from "./App";
import { testConfig, testStatus } from "./test/fixtures";

function jsonResponse(body: unknown) {
  return new Response(JSON.stringify(body), {
    status: 200,
    headers: { "Content-Type": "application/json" },
  });
}

function errorResponse(message: string, status = 500) {
  return new Response(JSON.stringify({ message }), {
    status,
    headers: { "Content-Type": "application/json" },
  });
}

type PutOverrides = Record<string, unknown>;

function buildApiFetchMock(putOverrides: PutOverrides = {}) {
  const calls: string[] = [];

  vi.spyOn(globalThis, "fetch").mockImplementation(async (input, init) => {
    const url = typeof input === "string" ? input : input.toString();
    const path = new URL(url, "http://device.local").pathname;
    const method = (init?.method ?? "GET").toUpperCase();
    const override = putOverrides[path];

    calls.push(`${method} ${path}`);

    if (method === "GET") {
      switch (path) {
        case "/api/v1/status":
          return jsonResponse(testStatus);
        case "/api/v1/config/station":
          return jsonResponse(testConfig.station);
        case "/api/v1/config/sampling":
          return jsonResponse(testConfig.sampling);
        case "/api/v1/config/smoothing":
          return jsonResponse(testConfig.smoothing);
        case "/api/v1/config/storage":
          return jsonResponse(testConfig.storage);
        case "/api/v1/config/network":
          return jsonResponse(testConfig.network);
        case "/api/v1/config/sms-admin":
          return jsonResponse(testConfig.smsAdmin);
        case "/api/v1/config/web-ui":
          return jsonResponse(testConfig.webUi);
        case "/api/v1/config/sensors":
          return jsonResponse(testConfig.sensors);
        case "/api/v1/config/publishers/wunderground":
          return jsonResponse(testConfig.publishers.wunderground);
        case "/api/v1/config/publishers/windy":
          return jsonResponse(testConfig.publishers.windy);
        case "/api/v1/config/publishers/mqtt":
          return jsonResponse(testConfig.publishers.mqtt);
        default:
          throw new Error(`Unexpected GET ${path}`);
      }
    }

    if (method === "PUT") {
      const body = init?.body ? JSON.parse(String(init.body)) : undefined;

      switch (path) {
        case "/api/v1/config/station":
          return jsonResponse((override as object | undefined) ?? { ...testConfig.station, ...body });
        case "/api/v1/config/sampling":
          return jsonResponse((override as object | undefined) ?? { ...testConfig.sampling, ...body });
        case "/api/v1/config/smoothing":
          return jsonResponse((override as object | undefined) ?? { ...testConfig.smoothing, ...body });
        case "/api/v1/config/storage":
          return jsonResponse((override as object | undefined) ?? { ...testConfig.storage, ...body });
        case "/api/v1/config/network":
          return jsonResponse((override as object | undefined) ?? { ...testConfig.network, ...body });
        case "/api/v1/config/sms-admin":
          return jsonResponse((override as object | undefined) ?? { ...testConfig.smsAdmin, ...body });
        case "/api/v1/config/web-ui":
          return jsonResponse((override as object | undefined) ?? { ...testConfig.webUi, ...body });
        case "/api/v1/config/sensors":
          return jsonResponse(override ?? body ?? testConfig.sensors);
        case "/api/v1/config/publishers/wunderground":
          return jsonResponse(
            (override as object | undefined) ?? { ...testConfig.publishers.wunderground, ...body },
          );
        case "/api/v1/config/publishers/windy":
          return jsonResponse((override as object | undefined) ?? { ...testConfig.publishers.windy, ...body });
        case "/api/v1/config/publishers/mqtt":
          return jsonResponse((override as object | undefined) ?? { ...testConfig.publishers.mqtt, ...body });
        default:
          throw new Error(`Unexpected PUT ${path}`);
      }
    }

    throw new Error(`Unexpected ${method} ${path}`);
  });

  return { calls };
}

function renderApp() {
  return render(
    <MantineProvider>
      <App />
    </MantineProvider>,
  );
}

async function waitForInitialConfigLoad(calls: string[]) {
  await waitFor(() => {
    expect(calls.filter((call) => call.startsWith("GET /api/v1/config/"))).toHaveLength(11);
  });
}

describe("App config flow", () => {
  beforeEach(() => {
    vi.restoreAllMocks();
    window.localStorage.clear();
    window.localStorage.setItem(
      "guardian.apiConnection",
      JSON.stringify({
        baseUrl: "http://device.local",
        apiKey: "secret",
      }),
    );
  });

  it("does not call the backend until the connection is configured", async () => {
    window.localStorage.clear();
    const fetchMock = vi.spyOn(globalThis, "fetch");

    renderApp();

    expect(await screen.findByText(/backend connection required/i)).toBeTruthy();
    expect(fetchMock).not.toHaveBeenCalled();
    expect((screen.getByRole("button", { name: /saved/i }) as HTMLButtonElement).disabled).toBe(true);
  });

  it("submits only dirty sections", async () => {
    const user = userEvent.setup();
    const { calls } = buildApiFetchMock();

    renderApp();

    const stationName = await screen.findByLabelText(/station name/i);
    await user.clear(stationName);
    await user.type(stationName, "New launch name");

    const saveButton = await screen.findByRole("button", { name: /save changes/i });
    await user.click(saveButton);

    await waitFor(() => {
      const putCalls = calls.filter((call) => call.startsWith("PUT "));
      expect(putCalls).toEqual(["PUT /api/v1/config/station"]);
    });
  });

  it("submits multiple dirty sections in sequence", async () => {
    const user = userEvent.setup();
    const { calls } = buildApiFetchMock();

    renderApp();

    const stationName = await screen.findByLabelText(/station name/i);
    await user.clear(stationName);
    await user.type(stationName, "New launch name");

    const deepSleepSwitch = await screen.findByLabelText(/deep sleep enabled/i);
    await user.click(deepSleepSwitch);

    const saveButton = await screen.findByRole("button", { name: /save changes/i });
    await user.click(saveButton);

    await waitFor(() => {
      const putCalls = calls.filter((call) => call.startsWith("PUT "));
      expect(putCalls).toEqual([
        "PUT /api/v1/config/station",
        "PUT /api/v1/config/sampling",
      ]);
    });
  });

  it("blocks submit when validation fails", async () => {
    const user = userEvent.setup();
    const { calls } = buildApiFetchMock();

    renderApp();

    const intervalInput = await screen.findByLabelText(/sample interval/i);
    await user.clear(intervalInput);
    await user.type(intervalInput, "999");
    fireEvent.blur(intervalInput);

    const saveButton = await screen.findByRole("button", { name: /save changes/i });
    await user.click(saveButton);

    expect(
      await screen.findByText(/please fix the highlighted configuration values before saving/i),
    ).toBeTruthy();
    expect(calls.filter((call) => call.startsWith("PUT "))).toHaveLength(0);
  });

  it("uses PUT responses to update the form without refetching config", async () => {
    const user = userEvent.setup();
    const updatedStation = {
      ...testConfig.station,
      stationName: "Server-accepted station name",
    } satisfies UiConfig["station"];
    const { calls } = buildApiFetchMock({
      "/api/v1/config/station": updatedStation,
    });

    renderApp();

    const stationName = (await screen.findByLabelText(/station name/i)) as HTMLInputElement;
    await waitForInitialConfigLoad(calls);
    const initialGetCount = calls.filter((call) => call.startsWith("GET /api/v1/config/")).length;

    await user.clear(stationName);
    await user.type(stationName, "Locally edited name");

    const saveButton = await screen.findByRole("button", { name: /save changes/i });
    await user.click(saveButton);

    await waitFor(() => {
      expect(stationName.value).toBe("Server-accepted station name");
    });

    const finalGetCount = calls.filter((call) => call.startsWith("GET /api/v1/config/")).length;
    expect(finalGetCount).toBe(initialGetCount);
    expect(calls.filter((call) => call === "PUT /api/v1/config/station")).toHaveLength(1);
  });

  it("shows a save error and stops later section saves after a failure", async () => {
    const user = userEvent.setup();
    const calls: string[] = [];

    vi.spyOn(globalThis, "fetch").mockImplementation(async (input, init) => {
      const url = typeof input === "string" ? input : input.toString();
      const path = new URL(url, "http://device.local").pathname;
      const method = (init?.method ?? "GET").toUpperCase();
      calls.push(`${method} ${path}`);

      if (method === "GET") {
        switch (path) {
          case "/api/v1/status":
            return jsonResponse(testStatus);
          case "/api/v1/config/station":
            return jsonResponse(testConfig.station);
          case "/api/v1/config/sampling":
            return jsonResponse(testConfig.sampling);
          case "/api/v1/config/smoothing":
            return jsonResponse(testConfig.smoothing);
          case "/api/v1/config/storage":
            return jsonResponse(testConfig.storage);
          case "/api/v1/config/network":
            return jsonResponse(testConfig.network);
          case "/api/v1/config/sms-admin":
            return jsonResponse(testConfig.smsAdmin);
          case "/api/v1/config/web-ui":
            return jsonResponse(testConfig.webUi);
          case "/api/v1/config/sensors":
            return jsonResponse(testConfig.sensors);
          case "/api/v1/config/publishers/wunderground":
            return jsonResponse(testConfig.publishers.wunderground);
          case "/api/v1/config/publishers/windy":
            return jsonResponse(testConfig.publishers.windy);
          case "/api/v1/config/publishers/mqtt":
            return jsonResponse(testConfig.publishers.mqtt);
          default:
            throw new Error(`Unexpected GET ${path}`);
        }
      }

      if (method === "PUT") {
        if (path === "/api/v1/config/station") {
          return errorResponse("Station save failed");
        }

        if (path === "/api/v1/config/sampling") {
          return jsonResponse(testConfig.sampling);
        }

        throw new Error(`Unexpected PUT ${path}`);
      }

      throw new Error(`Unexpected ${method} ${path}`);
    });

    renderApp();

    const stationName = await screen.findByLabelText(/station name/i);
    await user.clear(stationName);
    await user.type(stationName, "Broken station");

    const deepSleepSwitch = await screen.findByLabelText(/deep sleep enabled/i);
    await user.click(deepSleepSwitch);

    const saveButton = await screen.findByRole("button", { name: /save changes/i });
    await user.click(saveButton);

    expect(await screen.findByText(/station save failed/i)).toBeTruthy();
    expect(calls.filter((call) => call === "PUT /api/v1/config/station")).toHaveLength(1);
    expect(calls.filter((call) => call === "PUT /api/v1/config/sampling")).toHaveLength(0);
  });

  it("shows a config load error and retries successfully", async () => {
    const calls: string[] = [];
    let stationAttempts = 0;

    vi.spyOn(globalThis, "fetch").mockImplementation(async (input, init) => {
      const url = typeof input === "string" ? input : input.toString();
      const path = new URL(url, "http://device.local").pathname;
      const method = (init?.method ?? "GET").toUpperCase();
      calls.push(`${method} ${path}`);

      if (method !== "GET") {
        throw new Error(`Unexpected ${method} ${path}`);
      }

      switch (path) {
        case "/api/v1/status":
          return jsonResponse(testStatus);
        case "/api/v1/config/station":
          stationAttempts += 1;
          return stationAttempts === 1
            ? errorResponse("Station config missing")
            : jsonResponse(testConfig.station);
        case "/api/v1/config/sampling":
          return jsonResponse(testConfig.sampling);
        case "/api/v1/config/smoothing":
          return jsonResponse(testConfig.smoothing);
        case "/api/v1/config/storage":
          return jsonResponse(testConfig.storage);
        case "/api/v1/config/network":
          return jsonResponse(testConfig.network);
        case "/api/v1/config/sms-admin":
          return jsonResponse(testConfig.smsAdmin);
        case "/api/v1/config/web-ui":
          return jsonResponse(testConfig.webUi);
        case "/api/v1/config/sensors":
          return jsonResponse(testConfig.sensors);
        case "/api/v1/config/publishers/wunderground":
          return jsonResponse(testConfig.publishers.wunderground);
        case "/api/v1/config/publishers/windy":
          return jsonResponse(testConfig.publishers.windy);
        case "/api/v1/config/publishers/mqtt":
          return jsonResponse(testConfig.publishers.mqtt);
        default:
          throw new Error(`Unexpected GET ${path}`);
      }
    });

    renderApp();

    expect(await screen.findByText(/could not load configuration/i)).toBeTruthy();

    const retryButtons = await screen.findAllByRole("button", { name: /retry/i });
    await userEvent.setup().click(retryButtons[0]);

    expect(await screen.findByLabelText(/station name/i)).toBeTruthy();
    expect(calls.filter((call) => call === "GET /api/v1/config/station")).toHaveLength(2);
  });

  it("shows a status load error and retries successfully", async () => {
    let statusAttempts = 0;

    vi.spyOn(globalThis, "fetch").mockImplementation(async (input, init) => {
      const url = typeof input === "string" ? input : input.toString();
      const path = new URL(url, "http://device.local").pathname;
      const method = (init?.method ?? "GET").toUpperCase();

      if (method !== "GET") {
        throw new Error(`Unexpected ${method} ${path}`);
      }

      switch (path) {
        case "/api/v1/status":
          statusAttempts += 1;
          return statusAttempts === 1
            ? errorResponse("Status unavailable")
            : jsonResponse(testStatus);
        case "/api/v1/config/station":
          return jsonResponse(testConfig.station);
        case "/api/v1/config/sampling":
          return jsonResponse(testConfig.sampling);
        case "/api/v1/config/smoothing":
          return jsonResponse(testConfig.smoothing);
        case "/api/v1/config/storage":
          return jsonResponse(testConfig.storage);
        case "/api/v1/config/network":
          return jsonResponse(testConfig.network);
        case "/api/v1/config/sms-admin":
          return jsonResponse(testConfig.smsAdmin);
        case "/api/v1/config/web-ui":
          return jsonResponse(testConfig.webUi);
        case "/api/v1/config/sensors":
          return jsonResponse(testConfig.sensors);
        case "/api/v1/config/publishers/wunderground":
          return jsonResponse(testConfig.publishers.wunderground);
        case "/api/v1/config/publishers/windy":
          return jsonResponse(testConfig.publishers.windy);
        case "/api/v1/config/publishers/mqtt":
          return jsonResponse(testConfig.publishers.mqtt);
        default:
          throw new Error(`Unexpected GET ${path}`);
      }
    });

    renderApp();

    expect(await screen.findByText(/could not load device status/i)).toBeTruthy();

    const retryButtons = await screen.findAllByRole("button", { name: /retry/i });
    await userEvent.setup().click(retryButtons[0]);

    await waitFor(() => {
      expect(screen.queryByText(/could not load device status/i)).toBeNull();
    });
    expect(statusAttempts).toBe(2);
  });
});
