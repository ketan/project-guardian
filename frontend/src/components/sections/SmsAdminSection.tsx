import { Badge, Card, Group, Stack, Switch, Text } from "@mantine/core";
import { SectionCard } from "../SectionCard";
import type { UiConfig } from "../../types/ui";

type SmsAdminSectionProps = {
  config: UiConfig;
  setConfig: React.Dispatch<React.SetStateAction<UiConfig>>;
};

export function SmsAdminSection({ config, setConfig }: SmsAdminSectionProps) {
  return (
    <SectionCard
      id="sms"
      title="SMS administration"
      subtitle="Trusted numbers can wake the station and manage connectivity without a password."
    >
      <Stack gap="md">
        <Switch
          checked={config.smsAdmin.enabled}
          onChange={(event) =>
            setConfig((current) => ({
              ...current,
              smsAdmin: { ...current.smsAdmin, enabled: event.currentTarget.checked },
            }))
          }
          label="SMS admin enabled"
        />

        {config.smsAdmin.whitelist.map((entry) => (
          <Card key={entry.phoneNumber} withBorder padding="md">
            <Stack gap={2}>
              <Text fw={700}>{entry.label}</Text>
              <Text size="sm" c="dimmed">
                {entry.phoneNumber}
              </Text>
            </Stack>
          </Card>
        ))}

        <Group gap="xs">
          {config.smsAdmin.commands.map((command) => (
            <Badge key={command} variant="outline" radius="sm">
              {command.replaceAll("_", " ")}
            </Badge>
          ))}
        </Group>
      </Stack>
    </SectionCard>
  );
}
