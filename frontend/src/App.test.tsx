import { fireEvent, render, screen, waitFor } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { beforeEach, describe, expect, it, vi } from "vitest";
import { MantineProvider } from "@mantine/core";
import App from "./App";
import { testConfig, testStatus } from "./test/fixtures";

function jsonResponse(body: unknown) {
  return new Response(JSON.stringify(body), {
    status: 200,
    headers: { "Content-Type": "application/json" },
  });
}

function buildApiFetchMock() {
  const calls: string[] = [];

  const fetchMock = vi.spyOn(globalThis, "fetch").mockImplementation(async (input, init) => {
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
      const body = init?.body ? JSON.parse(String(init.body)) : undefined;

      switch (path) {
        case "/api/v1/config/station":
          return jsonResponse({ ...testConfig.station, ...body });
        case "/api/v1/config/sampling":
          return jsonResponse({ ...testConfig.sampling, ...body });
        case "/api/v1/config/smoothing":
          return jsonResponse({ ...testConfig.smoothing, ...body });
        case "/api/v1/config/storage":
          return jsonResponse({ ...testConfig.storage, ...body });
        case "/api/v1/config/network":
          return jsonResponse({ ...testConfig.network, ...body });
        case "/api/v1/config/sms-admin":
          return jsonResponse({ ...testConfig.smsAdmin, ...body });
        case "/api/v1/config/web-ui":
          return jsonResponse({ ...testConfig.webUi, ...body });
        case "/api/v1/config/sensors":
          return jsonResponse(body ?? testConfig.sensors);
        case "/api/v1/config/publishers/wunderground":
          return jsonResponse({ ...testConfig.publishers.wunderground, ...body });
        case "/api/v1/config/publishers/windy":
          return jsonResponse({ ...testConfig.publishers.windy, ...body });
        case "/api/v1/config/publishers/mqtt":
          return jsonResponse({ ...testConfig.publishers.mqtt, ...body });
        default:
          throw new Error(`Unexpected PUT ${path}`);
      }
    }

    throw new Error(`Unexpected ${method} ${path}`);
  });

  return { calls, fetchMock };
}

function renderApp() {
  return render(
    <MantineProvider>
      <App />
    </MantineProvider>,
  );
}

describe("App config flow", () => {
  beforeEach(() => {
    window.localStorage.clear();
    window.localStorage.setItem(
      "guardian.apiConnection",
      JSON.stringify({
        baseUrl: "http://device.local",
        apiKey: "secret",
      }),
    );
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
});
