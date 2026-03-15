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
import { FiPlus, FiTrash2 } from "react-icons/fi";
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
  setConfig: React.Dispatch<React.SetStateAction<UiConfig>>;
};

export function SensorsSection({ config, setConfig }: SensorsSectionProps) {
  const updateSensor = (index: number, updates: Partial<SensorConfig>) => {
    setConfig((current) => ({
      ...current,
      sensors: current.sensors.map((sensor, sensorIndex) =>
        sensorIndex === index ? { ...sensor, ...updates } : sensor,
      ),
    }));
  };

  const removeSensor = (index: number) => {
    setConfig((current) => ({
      ...current,
      sensors: current.sensors.filter((_, sensorIndex) => sensorIndex !== index),
    }));
  };

  const addSensor = () => {
    setConfig((current) => ({
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
      subtitle="Configure attached sensor modules and their connection settings."
      action={
        <Button leftSection={<FiPlus />} size="compact-sm" variant="light" onClick={addSensor}>
          Add sensor
        </Button>
      }
    >
      <Stack gap="md">
        {config.sensors.map((sensor, index) => (
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
                  <FiTrash2 />
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
