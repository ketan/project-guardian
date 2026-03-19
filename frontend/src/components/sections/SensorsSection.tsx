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
  TextInput,
} from "@mantine/core";
import { IconCpu, IconPlus, IconTrash } from "@tabler/icons-react";
import { SectionCard } from "../SectionCard";
import type { UiConfig } from "../../types/ui";

type SensorConfig = UiConfig["sensors"][number];

const sensorTypeOptions = [
  { value: "sen0658", label: "DFRobot SEN0658" },
  { value: "bme280", label: "BME280" },
  { value: "bmp390", label: "BMP390" },
  { value: "lps22hb", label: "LPS22HB" },
] as const;

const transportOptions = [
  { value: "rs485_modbus", label: "RS485 / Modbus" },
  { value: "i2c", label: "I2C" },
  { value: "spi", label: "SPI" },
] as const;

type SensorsSectionProps = {
  config: UiConfig;
  loading?: boolean;
  setConfig: React.Dispatch<React.SetStateAction<UiConfig>>;
};

export function SensorsSection({ config, loading = false, setConfig }: SensorsSectionProps) {
  const updateSensor = (index: number, updates: Partial<SensorConfig>) => {
    setConfig((current: UiConfig) => ({
      ...current,
      sensors: current.sensors.map((sensor: SensorConfig, sensorIndex: number) =>
        sensorIndex === index ? { ...sensor, ...updates } : sensor,
      ),
    }));
  };

  const removeSensor = (index: number) => {
    setConfig((current: UiConfig) => ({
      ...current,
      sensors: current.sensors.filter((_: SensorConfig, sensorIndex: number) => sensorIndex !== index),
    }));
  };

  const addSensor = () => {
    setConfig((current: UiConfig) => ({
      ...current,
      sensors: [
        ...current.sensors,
        {
          id: `sensor-${current.sensors.length + 1}`,
          type: "bme280",
          enabled: false,
          transport: "i2c",
        },
      ],
    }));
  };

  return (
    <SectionCard
      id="sensors"
      title="Sensors"
      icon={<IconCpu size={18} stroke={1.75} />}
      loading={loading}
      subtitle="Configure attached sensor modules and their connection settings."
      action={
        <Button leftSection={<IconPlus size={16} stroke={1.75} />} size="compact-sm" variant="light" onClick={addSensor}>
          Add sensor
        </Button>
      }
    >
      <Stack gap="md">
        {config.sensors.map((sensor: SensorConfig, index: number) => (
          <Card key={sensor.id} withBorder padding="lg">
            <Stack gap="md">
              <Group justify="space-between" align="center">
                <Text fw={700}>{sensor.id}</Text>
                <ActionIcon
                  color="red"
                  variant="light"
                  onClick={() => removeSensor(index)}
                  aria-label={`Remove sensor ${sensor.id}`}
                >
                    <IconTrash size={16} stroke={1.75} />
                  </ActionIcon>
              </Group>

              <Switch
                checked={sensor.enabled}
                label="Enabled"
                onChange={(event) => updateSensor(index, { enabled: event.currentTarget.checked })}
              />

              <Grid>
                <Grid.Col span={{ base: 12, sm: 6 }}>
                  <TextInput
                    label="Sensor ID"
                    value={sensor.id}
                    onChange={(event) => updateSensor(index, { id: event.currentTarget.value })}
                  />
                </Grid.Col>
                <Grid.Col span={{ base: 12, sm: 6 }}>
                  <Select
                    label="Sensor type"
                    data={sensorTypeOptions}
                    value={sensor.type}
                    onChange={(value) =>
                      value ? updateSensor(index, { type: value as SensorConfig["type"] }) : undefined
                    }
                  />
                </Grid.Col>
                <Grid.Col span={{ base: 12, sm: 6 }}>
                  <Select
                    label="Transport"
                    data={transportOptions}
                    value={sensor.transport ?? null}
                    onChange={(value) =>
                      updateSensor(index, {
                        transport: value ? (value as SensorConfig["transport"]) : undefined,
                      })
                    }
                  />
                </Grid.Col>
                <Grid.Col span={{ base: 12, sm: 6 }}>
                  <NumberInput
                    label="Poll interval (sec)"
                    min={1}
                    value={typeof sensor.pollIntervalSeconds === "number" ? sensor.pollIntervalSeconds : ""}
                    onChange={(value) =>
                      updateSensor(index, {
                        pollIntervalSeconds: typeof value === "number" ? value : undefined,
                      })
                    }
                  />
                </Grid.Col>
                <Grid.Col span={{ base: 12, sm: 6 }}>
                  <NumberInput
                    label="Modbus address"
                    min={1}
                    max={247}
                    value={typeof sensor.address === "number" ? sensor.address : ""}
                    onChange={(value) =>
                      updateSensor(index, { address: typeof value === "number" ? value : undefined })
                    }
                  />
                </Grid.Col>
              </Grid>
            </Stack>
          </Card>
        ))}
      </Stack>
    </SectionCard>
  );
}
