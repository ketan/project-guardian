import { Badge, NavLink, Paper, Stack, Text, Title } from "@mantine/core";
import type { DeviceStatus, NavItem } from "../types/ui";

type NavigationPanelProps = {
  items: NavItem[];
  status: DeviceStatus;
  onNavigate?: () => void;
};

export function NavigationPanel({ items, status, onNavigate }: NavigationPanelProps) {
  return (
    <Stack gap="md" p="md">
      <Paper p="lg" bg="dark.8" c="white">
        <Stack gap="xs">
          <Text tt="uppercase" size="xs" fw={700} c="teal.2">
            Project Guardian
          </Text>
          <Title order={4} c="white">
            Weather Station Console
          </Title>
          <Text size="sm" c="gray.3">
            Mobile-first controls for station health, publishers, and pilot-facing weather ops.
          </Text>
        </Stack>
      </Paper>

      <Stack gap={6}>
        {items.map((item) => (
          <NavLink
            key={item.id}
            component="a"
            href={`#${item.id}`}
            onClick={onNavigate}
            label={item.label}
            leftSection={item.icon}
            variant="light"
            color="teal"
            styles={{
              root: { borderRadius: 10 },
              section: { fontSize: 18 },
            }}
          />
        ))}
      </Stack>

      <Paper p="lg" bg="teal.6" c="white">
        <Stack gap="sm">
          <Text fw={700}>Remote admin access</Text>
          <Text size="sm" c="teal.0">
            SMS `OPEN SESAME` from a whitelisted number to bring the station online.
          </Text>
          <Badge
            color={status.adminWindow.active ? "lime" : "gray"}
            variant="light"
            radius="sm"
            w="fit-content"
          >
            {status.adminWindow.active ? "Window active" : "Window closed"}
          </Badge>
        </Stack>
      </Paper>
    </Stack>
  );
}
