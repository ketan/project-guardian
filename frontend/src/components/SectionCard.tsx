import type { ReactNode } from "react";
import { Group, Paper, Stack, Text, ThemeIcon, Title } from "@mantine/core";

type SectionCardProps = {
  id: string;
  title: string;
  subtitle?: string;
  icon?: ReactNode;
  action?: ReactNode;
  children: ReactNode;
};

export function SectionCard({ id, title, subtitle, icon, action, children }: SectionCardProps) {
  return (
    <Paper id={id} withBorder p={{ base: "lg", sm: "xl" }} shadow="sm">
      <Stack gap="lg">
        <Group justify="space-between" align="flex-start">
          <Stack gap={4}>
            <Group gap="xs" align="center">
              {icon ? (
                <ThemeIcon color="gray" size="md" variant="light" radius="xl">
                  {icon}
                </ThemeIcon>
              ) : null}
              <Title order={3}>{title}</Title>
            </Group>
            {subtitle ? (
              <Text c="dimmed" size="sm">
                {subtitle}
              </Text>
            ) : null}
          </Stack>
          {action}
        </Group>
        {children}
      </Stack>
    </Paper>
  );
}
