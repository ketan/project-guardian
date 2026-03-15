import { Badge, Card, Divider, Group, Stack, Text } from "@mantine/core";
import { FiCpu } from "react-icons/fi";
import { SectionCard } from "../SectionCard";
import type { DeviceStatus } from "../../types/ui";

type HealthSectionProps = {
  status: DeviceStatus;
};

export function HealthSection({ status }: HealthSectionProps) {
  return (
    <SectionCard
      id="overview-health"
      title="Sensor and storage health"
      subtitle="Quick status snapshot for launch-day confidence checks."
    >
      <Stack gap="md">
        {status.sensors.map((sensor: DeviceStatus["sensors"][number]) => (
          <Card key={sensor.id} withBorder padding="md">
            <Group justify="space-between" align="center">
              <Stack gap={2}>
                <Text fw={700}>{sensor.kind}</Text>
                <Text c="dimmed" size="sm">
                  {sensor.message}
                </Text>
              </Stack>
              <Badge
                leftSection={<FiCpu />}
                color={sensor.healthy ? "green" : "yellow"}
                variant="light"
                radius="sm"
              >
                {sensor.healthy ? "Healthy" : "Attention"}
              </Badge>
            </Group>
          </Card>
        ))}

        <Divider />

        <Text c="dimmed" size="sm">
          Recent and history endpoints will stream stored JSON directly from SD card, which keeps
          memory usage predictable even on long retention windows.
        </Text>
      </Stack>
    </SectionCard>
  );
}
