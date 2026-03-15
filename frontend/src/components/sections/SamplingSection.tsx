import { Grid, NumberInput, Select, Stack, Switch, Text } from "@mantine/core";
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
          <Select
            label="History retention"
            data={Array.from({ length: 30 }, (_, index) => ({
              value: String(index + 1),
              label: `${index + 1} day${index === 0 ? "" : "s"}`,
            }))}
            value={String(config.storage.retentionDays)}
            onChange={(value) =>
              value
                ? setConfig((current: UiConfig) => ({
                    ...current,
                    storage: { ...current.storage, retentionDays: Number(value) },
                  }))
                : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 6 }}>
          <Select
            label="History aggregation"
            data={[
              { value: "5", label: "5 min" },
              { value: "15", label: "15 min" },
              { value: "30", label: "30 min" },
              { value: "60", label: "60 min" },
            ]}
            value={String(config.sampling.historyAggregationMinutes ?? 30)}
            onChange={(value) =>
              value ? updateSamplingField("historyAggregationMinutes", Number(value)) : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 6 }}>
          <Select
            label="Log format"
            data={[{ value: "jsonl", label: "JSONL" }]}
            value={config.storage.logFormat}
            onChange={(value) =>
              value
                ? setConfig((current: UiConfig) => ({
                    ...current,
                    storage: { ...current.storage, logFormat: value as UiConfig["storage"]["logFormat"] },
                  }))
                : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 6 }}>
          <Select
            label="Config source"
            data={[{ value: "sd_with_flash_fallback", label: "SD with flash fallback" }]}
            value={config.storage.configSource ?? "sd_with_flash_fallback"}
            onChange={(value) =>
              value
                ? setConfig((current: UiConfig) => ({
                    ...current,
                    storage: {
                      ...current.storage,
                      configSource: value as UiConfig["storage"]["configSource"],
                    },
                  }))
                : undefined
            }
          />
        </Grid.Col>
      </Grid>

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
    </SectionCard>
  );
}
