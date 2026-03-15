import { Divider, Grid, Group, NumberInput, Stack, Switch, Text } from "@mantine/core";
import { SectionCard } from "../SectionCard";
import type { DeviceConfig, UiConfig } from "../../types/ui";

type SamplingSectionProps = {
  config: UiConfig;
  updateSamplingField: <K extends keyof DeviceConfig["sampling"]>(
    key: K,
    value: DeviceConfig["sampling"][K],
  ) => void;
  setConfig: React.Dispatch<React.SetStateAction<UiConfig>>;
};

export function SamplingSection({
  config,
  updateSamplingField,
  setConfig,
}: SamplingSectionProps) {
  return (
    <SectionCard
      id="sampling"
      title="Sampling and retention"
      subtitle="Compact controls for timing, sleep, and on-device history."
    >
      <Grid>
        <Grid.Col span={{ base: 12, sm: 4 }}>
          <NumberInput
            label="Sample interval"
            suffix=" sec"
            value={config.sampling.intervalSeconds}
            onChange={(value) =>
              typeof value === "number" ? updateSamplingField("intervalSeconds", value) : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 4 }}>
          <NumberInput
            label="Wake duration"
            suffix=" sec"
            value={config.sampling.wakeDurationSeconds ?? 0}
            onChange={(value) =>
              typeof value === "number" ? updateSamplingField("wakeDurationSeconds", value) : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 4 }}>
          <NumberInput
            label="Admin window"
            suffix=" min"
            value={config.sampling.adminWindowMinutes}
            onChange={(value) =>
              typeof value === "number" ? updateSamplingField("adminWindowMinutes", value) : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 6 }}>
          <NumberInput
            label="History retention"
            suffix=" days"
            value={config.storage.retentionDays}
            onChange={(value) =>
              typeof value === "number"
                ? setConfig((current) => ({
                    ...current,
                    storage: { ...current.storage, retentionDays: value },
                  }))
                : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 6 }}>
          <NumberInput
            label="History aggregation"
            suffix=" min"
            value={config.sampling.historyAggregationMinutes ?? 0}
            onChange={(value) =>
              typeof value === "number"
                ? updateSamplingField("historyAggregationMinutes", value)
                : undefined
            }
          />
        </Grid.Col>
      </Grid>

      <Divider />

      <Group grow align="flex-start">
        <Stack gap={2}>
          <Switch
            checked={config.sampling.deepSleepEnabled}
            onChange={(event) => updateSamplingField("deepSleepEnabled", event.currentTarget.checked)}
            label="Deep sleep enabled"
          />
          <Text c="dimmed" size="sm">
            Let the ESP32 sleep between samples to preserve battery and reduce thermal noise.
          </Text>
        </Stack>
        <Stack gap={2}>
          <Switch
            checked={config.smoothing.enabled}
            onChange={(event) =>
              setConfig((current) => ({
                ...current,
                smoothing: { ...current.smoothing, enabled: event.currentTarget.checked },
              }))
            }
            label="Smoothing enabled"
          />
          <Text c="dimmed" size="sm">
            Keep short-term spikes from dominating charts and downstream weather-site uploads.
          </Text>
        </Stack>
      </Group>
    </SectionCard>
  );
}
