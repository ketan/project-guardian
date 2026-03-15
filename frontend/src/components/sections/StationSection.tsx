import { Badge, Grid, NumberInput, TextInput, Textarea } from "@mantine/core";
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
        <Grid.Col span={{ base: 12, sm: 6 }}>
          <TextInput
            label="Station name"
            value={config.station.stationName}
            onChange={(event) => updateStationField("stationName", event.currentTarget.value)}
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 6 }}>
          <TextInput
            label="Timezone"
            value={config.station.timezone}
            onChange={(event) => updateStationField("timezone", event.currentTarget.value)}
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 4 }}>
          <NumberInput
            label="Latitude"
            value={config.station.location.latitude}
            decimalScale={5}
            onChange={(value) =>
              typeof value === "number"
                ? setConfig((current) => ({
                    ...current,
                    station: {
                      ...current.station,
                      location: { ...current.station.location, latitude: value },
                    },
                  }))
                : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 4 }}>
          <NumberInput
            label="Longitude"
            value={config.station.location.longitude}
            decimalScale={5}
            onChange={(value) =>
              typeof value === "number"
                ? setConfig((current) => ({
                    ...current,
                    station: {
                      ...current.station,
                      location: { ...current.station.location, longitude: value },
                    },
                  }))
                : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 4 }}>
          <NumberInput
            label="Elevation (m)"
            value={config.station.location.elevationMeters ?? 0}
            onChange={(value) =>
              typeof value === "number"
                ? setConfig((current) => ({
                    ...current,
                    station: {
                      ...current.station,
                      location: { ...current.station.location, elevationMeters: value },
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
