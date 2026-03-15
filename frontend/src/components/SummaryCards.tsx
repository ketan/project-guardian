import type { ReactNode } from "react";
import { Group, Paper, SimpleGrid, Stack, Text, ThemeIcon } from "@mantine/core";
import { FiActivity, FiHardDrive, FiRadio, FiSend } from "react-icons/fi";
import type { DeviceStatus } from "../types/ui";

type SummaryCard = {
  label: string;
  value: string;
  detail?: string;
  icon: ReactNode;
};

type SummaryCardsProps = {
  status: DeviceStatus;
};

export function SummaryCards({ status }: SummaryCardsProps) {
  const cards: SummaryCard[] = [
    {
      label: "Current transport",
      value: status.connectivity.activeTransport.toUpperCase(),
      detail: status.connectivity.wifi.connected
        ? status.connectivity.wifi.ssid
        : status.connectivity.cellular.operatorName,
      icon: <FiRadio />,
    },
    {
      label: "Sampling",
      value: `${status.sampling.intervalSeconds}s`,
      detail: status.sampling.sleepEnabled ? "Deep sleep enabled" : "Always awake",
      icon: <FiActivity />,
    },
    {
      label: "Retention",
      value: `${status.storage.retentionDays} days`,
      detail: status.storage.sdCardPresent ? "microSD healthy" : "No SD detected",
      icon: <FiHardDrive />,
    },
    {
      label: "Publishers",
      value: `${status.publishers.filter((item) => item.enabled).length} active`,
      detail: "Windy + Meshtastic MQTT",
      icon: <FiSend />,
    },
  ];

  return (
    <SimpleGrid cols={{ base: 1, sm: 2, xl: 4 }} spacing="md">
      {cards.map((card) => (
        <Paper key={card.label} withBorder p="lg" shadow="sm">
          <Group justify="space-between" align="flex-start" wrap="nowrap">
            <Stack gap={4}>
              <Text c="dimmed" size="sm">
                {card.label}
              </Text>
              <Text fw={800} size="2rem">
                {card.value}
              </Text>
              <Text c="dimmed" size="sm">
                {card.detail}
              </Text>
            </Stack>
            <ThemeIcon size={44} variant="light" color="teal">
              {card.icon}
            </ThemeIcon>
          </Group>
        </Paper>
      ))}
    </SimpleGrid>
  );
}
