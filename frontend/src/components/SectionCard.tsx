import type { ReactNode } from "react";
import { Group, Paper, Stack, Text, Title } from "@mantine/core";

type SectionCardProps = {
  id: string;
  title: string;
  subtitle?: string;
  action?: ReactNode;
  children: ReactNode;
};

export function SectionCard({ id, title, subtitle, action, children }: SectionCardProps) {
  return (
    <Paper id={id} withBorder p={{ base: "lg", sm: "xl" }} shadow="sm">
      <Stack gap="lg">
        <Group justify="space-between" align="flex-start">
          <Stack gap={4}>
            <Title order={3}>{title}</Title>
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
