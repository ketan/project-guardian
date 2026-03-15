import { Badge, Grid, NumberInput, SegmentedControl, Stack, Text, TextInput, Textarea } from "@mantine/core";
import { SectionCard } from "../SectionCard";
import type { DeviceConfig, UiConfig } from "../../types/ui";

type StationSectionProps = {
  config: UiConfig;
  updateStationField: <K extends keyof DeviceConfig["station"]>(
    key: K,
    value: DeviceConfig["station"][K],
  ) => void;
  setConfig: React.Dispatch<React.SetStateAction<UiConfig>>;
};

export function StationSection({ config, updateStationField, setConfig }: StationSectionProps) {
  const autoGps = config.station.locationFromGPS;
  const manualLocation = config.station.location ?? {
    latitude: 0,
    longitude: 0,
    elevationMeters: 0,
  };

  return (
    <SectionCard
      id="station"
      title="Station identity"
      subtitle="Core site metadata for public weather services and forecast comparison."
      action={
        <Badge color="teal" variant="light" radius="sm">{`Schema v${config.schemaVersion}`}</Badge>
      }
    >
      <Grid>
        <Grid.Col span={12}>
          <TextInput
            label="Station name"
            value={config.station.stationName}
            onChange={(event) => updateStationField("stationName", event.currentTarget.value)}
          />
        </Grid.Col>
        <Grid.Col span={12}>
          <Stack gap={6}>
            <Text fw={500} size="sm">
              Location source
            </Text>
            <SegmentedControl
              fullWidth
              value={config.station.locationFromGPS ? "gps" : "manual"}
              onChange={(value) => updateStationField("locationFromGPS", value === "gps")}
              data={[
                { label: "Manual", value: "manual" },
                { label: "Auto GPS", value: "gps" },
              ]}
            />
            <Text c="dimmed" size="sm">
              When enabled, the device uses the latest cached GPS fix for latitude, longitude, and
              elevation, and the manual fields stay disabled.
            </Text>
          </Stack>
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 4 }}>
          <NumberInput
            label="GPS refresh (hours)"
            value={config.station.gpsPollIntervalHours}
            min={1}
            max={24}
            disabled={!autoGps}
            onChange={(value) =>
              typeof value === "number"
                ? updateStationField("gpsPollIntervalHours", value)
                : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 8 }}>
          <Text c="dimmed" mt={30} size="sm">
            GPS is only polled on this interval to reduce power use. The device keeps using the
            last valid fix between refreshes.
          </Text>
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 4 }}>
          <NumberInput
            label="Latitude"
            value={manualLocation.latitude}
            decimalScale={5}
            disabled={autoGps}
            onChange={(value) =>
              typeof value === "number"
                ? setConfig((current: UiConfig) => ({
                    ...current,
                    station: {
                      ...current.station,
                      location: {
                        ...(current.station.location ?? manualLocation),
                        latitude: value,
                      },
                    },
                  }))
                : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 4 }}>
          <NumberInput
            label="Longitude"
            value={manualLocation.longitude}
            decimalScale={5}
            disabled={autoGps}
            onChange={(value) =>
              typeof value === "number"
                ? setConfig((current: UiConfig) => ({
                    ...current,
                    station: {
                      ...current.station,
                      location: {
                        ...(current.station.location ?? manualLocation),
                        longitude: value,
                      },
                    },
                  }))
                : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 4 }}>
          <NumberInput
            label="Elevation (m)"
            value={manualLocation.elevationMeters ?? 0}
            disabled={autoGps}
            onChange={(value) =>
              typeof value === "number"
                ? setConfig((current: UiConfig) => ({
                    ...current,
                    station: {
                      ...current.station,
                      location: {
                        ...(current.station.location ?? manualLocation),
                        elevationMeters: value,
                      },
                    },
                  }))
                : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={12}>
          <Textarea
            label="Notes"
            minRows={3}
            value={config.station.notes ?? ""}
            onChange={(event) => updateStationField("notes", event.currentTarget.value)}
          />
        </Grid.Col>
      </Grid>
    </SectionCard>
  );
}
