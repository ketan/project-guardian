import {Badge, NavLink, Paper, Stack, Text} from "@mantine/core";
import type {DeviceStatus, NavItem} from "../types/ui";

type NavigationPanelProps = {
  items: NavItem[];
  status: DeviceStatus | null;
  onNavigate?: () => void;
};

export function NavigationPanel({items, status, onNavigate}: NavigationPanelProps) {
  return (
    <Stack
      gap="sm"
      p="sm"
      h="100%"
      style={{
        overflowY: "auto",
        overscrollBehavior: "contain",
      }}
    >
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
              root: {borderRadius: 10},
              section: {fontSize: 18},
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
            color={status?.adminWindow.active ? "lime" : "gray"}
            variant="filled"
            radius="xl"
            w="fit-content"
          >
            {status?.adminWindow.active ? "Window active" : "Window closed"}
          </Badge>
        </Stack>
      </Paper>
    </Stack>
  );
}
