import {
  ActionIcon,
  Button,
  Card,
  Grid,
  Group,
  NumberInput,
  Select,
  Stack,
  Switch,
  Text,
} from "@mantine/core";
import { FiPlus, FiTrash2 } from "react-icons/fi";
import { SectionCard } from "../SectionCard";
import type { UiConfig } from "../../types/ui";

type SmoothingField = UiConfig["smoothing"]["fields"][number];

const metricOptions = [
  { value: "windSpeed", label: "Wind speed" },
  { value: "windGust", label: "Wind gust" },
  { value: "windDirection", label: "Wind direction" },
  { value: "temperature", label: "Temperature" },
  { value: "humidity", label: "Humidity" },
  { value: "pressure", label: "Pressure" },
  { value: "rainfall", label: "Rainfall" },
  { value: "illuminance", label: "Illuminance" },
  { value: "pm2_5", label: "PM2.5" },
  { value: "pm10", label: "PM10" },
  { value: "noise", label: "Noise" },
] as const;

const methodOptions = [
  { value: "none", label: "None" },
  { value: "moving_average", label: "Moving average" },
  { value: "ema", label: "EMA" },
] as const;

type SmoothingSectionProps = {
  config: UiConfig;
  setConfig: React.Dispatch<React.SetStateAction<UiConfig>>;
};

export function SmoothingSection({ config, setConfig }: SmoothingSectionProps) {
  const updateField = (index: number, updates: Partial<SmoothingField>) => {
    setConfig((current: UiConfig) => ({
      ...current,
      smoothing: {
        ...current.smoothing,
        fields: current.smoothing.fields.map((field: SmoothingField, fieldIndex: number) =>
          fieldIndex === index ? { ...field, ...updates } : field,
        ),
      },
    }));
  };

  const removeField = (index: number) => {
    setConfig((current: UiConfig) => ({
      ...current,
      smoothing: {
        ...current.smoothing,
        fields: current.smoothing.fields.filter((_: SmoothingField, fieldIndex: number) => fieldIndex !== index),
      },
    }));
  };

  const addField = () => {
    const usedMetrics = new Set(config.smoothing.fields.map((field: SmoothingField) => field.metric));
    const nextMetric =
      metricOptions.find((option) => !usedMetrics.has(option.value))?.value ?? "windSpeed";

    setConfig((current: UiConfig) => ({
      ...current,
      smoothing: {
        ...current.smoothing,
        fields: [
          ...current.smoothing.fields,
          { metric: nextMetric, method: "moving_average", windowSamples: 3 },
        ],
      },
    }));
  };

  return (
    <SectionCard
      id="smoothing"
      title="Smoothing"
      subtitle="Tune which measurements get averaged before charting and publishing."
      action={
        <Button leftSection={<FiPlus />} size="compact-sm" variant="light" onClick={addField}>
          Add rule
        </Button>
      }
    >
      <Stack gap="md">
        <Switch
          checked={config.smoothing.enabled}
          label="Smoothing enabled"
          onChange={(event) =>
            setConfig((current: UiConfig) => ({
              ...current,
              smoothing: { ...current.smoothing, enabled: event.currentTarget.checked },
            }))
          }
        />

        {config.smoothing.fields.map((field: SmoothingField, index: number) => (
          <Card key={`${field.metric}-${index}`} withBorder padding="lg">
            <Stack gap="md">
              <Group justify="space-between" align="center">
                <Text fw={700}>Rule {index + 1}</Text>
                <ActionIcon
                  color="red"
                  variant="light"
                  onClick={() => removeField(index)}
                  aria-label={`Remove smoothing rule ${index + 1}`}
                >
                  <FiTrash2 />
                </ActionIcon>
              </Group>

              <Grid>
                <Grid.Col span={{ base: 12, sm: 4 }}>
                  <Select
                    label="Metric"
                    data={metricOptions}
                    value={field.metric}
                    onChange={(value) =>
                      value ? updateField(index, { metric: value as SmoothingField["metric"] }) : undefined
                    }
                  />
                </Grid.Col>
                <Grid.Col span={{ base: 12, sm: 4 }}>
                  <Select
                    label="Method"
                    data={methodOptions}
                    value={field.method}
                    onChange={(value) =>
                      value ? updateField(index, { method: value as SmoothingField["method"] }) : undefined
                    }
                  />
                </Grid.Col>
                {field.method === "moving_average" ? (
                  <Grid.Col span={{ base: 12, sm: 4 }}>
                    <NumberInput
                      label="Window samples"
                      min={1}
                      value={field.windowSamples ?? 1}
                      onChange={(value) =>
                        typeof value === "number"
                          ? updateField(index, { windowSamples: value, alpha: undefined })
                          : undefined
                      }
                    />
                  </Grid.Col>
                ) : null}
                {field.method === "ema" ? (
                  <Grid.Col span={{ base: 12, sm: 4 }}>
                    <NumberInput
                      label="Alpha"
                      min={0}
                      max={1}
                      step={0.05}
                      decimalScale={2}
                      value={field.alpha ?? 0.25}
                      onChange={(value) =>
                        typeof value === "number"
                          ? updateField(index, { alpha: value, windowSamples: undefined })
                          : undefined
                      }
                    />
                  </Grid.Col>
                ) : null}
              </Grid>
            </Stack>
          </Card>
        ))}
      </Stack>
    </SectionCard>
  );
}
