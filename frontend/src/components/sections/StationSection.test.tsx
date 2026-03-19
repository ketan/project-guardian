import { render, screen } from "@testing-library/react";
import { MantineProvider } from "@mantine/core";
import { describe, expect, it, vi } from "vitest";
import { StationSection } from "./StationSection";
import { testConfig } from "../../test/fixtures";

function renderStationSection(locationFromGPS: boolean) {
  return render(
    <MantineProvider>
      <StationSection
        config={{
          ...testConfig,
          station: {
            ...testConfig.station,
            locationFromGPS,
          },
        }}
        updateStationField={vi.fn()}
        setConfig={vi.fn()}
      />
    </MantineProvider>,
  );
}

describe("StationSection", () => {
  it("disables manual location fields when GPS mode is enabled", () => {
    renderStationSection(true);

    expect((screen.getByLabelText(/latitude/i) as HTMLInputElement).disabled).toBe(true);
    expect((screen.getByLabelText(/longitude/i) as HTMLInputElement).disabled).toBe(true);
    expect((screen.getByLabelText(/altitude/i) as HTMLInputElement).disabled).toBe(true);
  });

  it("enables manual location fields when GPS mode is disabled", () => {
    renderStationSection(false);

    expect((screen.getByLabelText(/latitude/i) as HTMLInputElement).disabled).toBe(false);
    expect((screen.getByLabelText(/longitude/i) as HTMLInputElement).disabled).toBe(false);
    expect((screen.getByLabelText(/altitude/i) as HTMLInputElement).disabled).toBe(false);
  });
});
