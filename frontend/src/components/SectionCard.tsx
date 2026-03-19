import type { ReactNode } from "react";
import { Box, Group, LoadingOverlay, Paper, Stack, Text, ThemeIcon, Title } from "@mantine/core";

type SectionCardProps = {
  id: string;
  title: string;
  subtitle?: string;
  icon?: ReactNode;
  loading?: boolean;
  action?: ReactNode;
  children: ReactNode;
};

export function SectionCard({ id, title, subtitle, icon, loading = false, action, children }: SectionCardProps) {
  return (
    <Paper id={id} withBorder p={{ base: "lg", sm: "xl" }} shadow="sm" pos="relative">
      <LoadingOverlay
        visible={loading}
        zIndex={10}
        overlayProps={{ blur: 2, radius: "sm" }}
        loaderProps={{ size: "md" }}
      />
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
          <Group gap="sm" align="center">{action}</Group>
        </Group>
        <Box style={{ visibility: loading ? "hidden" : "visible" }}>{children}</Box>
      </Stack>
    </Paper>
  );
}
