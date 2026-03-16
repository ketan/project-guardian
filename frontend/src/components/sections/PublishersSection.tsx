import {
  Badge,
  Card,
  Grid,
  Group,
  NumberInput,
  PasswordInput,
  Stack,
  Switch,
  Text,
  TextInput,
} from "@mantine/core";
import { IconSend } from "@tabler/icons-react";
import { SectionCard } from "../SectionCard";
import type {
  PublisherSlotKey,
  UiMqttPublisher,
  UiPublishers,
  UiWebhookPublisher,
  UiWindyPublisher,
  UiWundergroundPublisher,
} from "../../api/contracts";

type PublishersSectionProps = {
  publishers: UiPublishers;
  updatePublisher: (slot: PublisherSlotKey, updates: Record<string, unknown>) => void;
};

type PublisherCardProps = {
  slot: PublisherSlotKey;
  title: string;
  subtitle: string;
  publisher:
    | UiWundergroundPublisher
    | UiWindyPublisher
    | UiWebhookPublisher
    | UiMqttPublisher;
  updatePublisher: (slot: PublisherSlotKey, updates: Record<string, unknown>) => void;
};

function PublisherCard({ slot, title, subtitle, publisher, updatePublisher }: PublisherCardProps) {
  return (
    <Card withBorder padding="lg">
      <Stack gap="md">
        <Group justify="space-between" align="flex-start">
          <Stack gap={0}>
            <Text fw={700}>{title}</Text>
            <Text size="sm" c="dimmed">
              {subtitle}
            </Text>
          </Stack>
          <Badge color={publisher.enabled ? "green" : "gray"} variant="light" radius="sm">
            {publisher.enabled ? "Enabled" : "Disabled"}
          </Badge>
        </Group>

        <Switch
          checked={publisher.enabled}
          label="Enabled"
          onChange={(event) =>
            updatePublisher(slot, { enabled: event.currentTarget.checked })
          }
        />

        <Grid>
          <Grid.Col span={{ base: 12, sm: 6 }}>
            <NumberInput
              label="Publish interval (seconds)"
              min={30}
              value={publisher.publishIntervalSeconds}
              onChange={(value) =>
                typeof value === "number"
                  ? updatePublisher(slot, { publishIntervalSeconds: value })
                  : undefined
              }
            />
          </Grid.Col>
          <Grid.Col span={{ base: 12, sm: 6 }}>
            <NumberInput
              label="History window (minutes)"
              min={1}
              placeholder="Optional"
              value={publisher.includeHistoryWindowMinutes ?? ""}
              onChange={(value) =>
                updatePublisher(slot, {
                  includeHistoryWindowMinutes: typeof value === "number" ? value : undefined,
                })
              }
            />
          </Grid.Col>

          {(publisher.type === "windy" || publisher.type === "wunderground") && (
            <>
              <Grid.Col span={{ base: 12, sm: 6 }}>
                <TextInput
                  label="Station ID"
                  value={publisher.stationId}
                  onChange={(event) =>
                    updatePublisher(slot, { stationId: event.currentTarget.value })
                  }
                />
              </Grid.Col>
              <Grid.Col span={{ base: 12, sm: 6 }}>
                <PasswordInput
                  label="API key"
                  placeholder={publisher.apiKeyConfigured ? "Configured" : "Enter API key"}
                  value={publisher.apiKey ?? ""}
                  onChange={(event) =>
                    updatePublisher(slot, { apiKey: event.currentTarget.value })
                  }
                />
              </Grid.Col>
            </>
          )}

          {publisher.type === "webhook" && (
            <>
              <Grid.Col span={12}>
                <TextInput
                  label="Webhook URL"
                  value={publisher.endpoint}
                  onChange={(event) =>
                    updatePublisher(slot, { endpoint: event.currentTarget.value })
                  }
                />
              </Grid.Col>
              <Grid.Col span={12}>
                <PasswordInput
                  label="Authorization header"
                  placeholder={
                    publisher.authHeaderConfigured ? "Configured" : "Optional bearer token"
                  }
                  value={publisher.authHeader ?? ""}
                  onChange={(event) =>
                    updatePublisher(slot, { authHeader: event.currentTarget.value })
                  }
                />
              </Grid.Col>
            </>
          )}

          {publisher.type === "mqtt" && (
            <>
              <Grid.Col span={12}>
                <TextInput
                  label="Broker URL"
                  value={publisher.brokerUrl}
                  onChange={(event) =>
                    updatePublisher(slot, { brokerUrl: event.currentTarget.value })
                  }
                />
              </Grid.Col>
              <Grid.Col span={{ base: 12, sm: 6 }}>
                <TextInput
                  label="Topic"
                  value={publisher.topic}
                  onChange={(event) =>
                    updatePublisher(slot, { topic: event.currentTarget.value })
                  }
                />
              </Grid.Col>
              <Grid.Col span={{ base: 12, sm: 6 }}>
                <TextInput
                  label="Username"
                  value={publisher.username}
                  onChange={(event) =>
                    updatePublisher(slot, { username: event.currentTarget.value })
                  }
                />
              </Grid.Col>
              <Grid.Col span={12}>
                <PasswordInput
                  label="Password"
                  placeholder={publisher.passwordConfigured ? "Configured" : "Enter password"}
                  value={publisher.password ?? ""}
                  onChange={(event) =>
                    updatePublisher(slot, { password: event.currentTarget.value })
                  }
                />
              </Grid.Col>
            </>
          )}
        </Grid>
      </Stack>
    </Card>
  );
}

export function PublishersSection({ publishers, updatePublisher }: PublishersSectionProps) {
  return (
    <SectionCard
      id="publishers"
      title="Publishers"
      icon={<IconSend size={18} stroke={1.75} />}
      subtitle="Configure where the station pushes weather updates and how often each destination is used."
    >
      <Stack gap="md">
        {publishers.windy ? (
          <PublisherCard
            slot="windy"
            title="Windy"
            subtitle="Push station observations to Windy for public forecast comparison."
            publisher={publishers.windy}
            updatePublisher={updatePublisher}
          />
        ) : null}
        {publishers.wunderground ? (
          <PublisherCard
            slot="wunderground"
            title="Weather Underground"
            subtitle="Send measurements to Weather Underground for broader historical visibility."
            publisher={publishers.wunderground}
            updatePublisher={updatePublisher}
          />
        ) : null}
        {publishers.webhook ? (
          <PublisherCard
            slot="webhook"
            title="Webhook"
            subtitle="Forward weather updates to a custom HTTPS endpoint."
            publisher={publishers.webhook}
            updatePublisher={updatePublisher}
          />
        ) : null}
        {publishers.mqtt ? (
          <PublisherCard
            slot="mqtt"
            title="Meshtastic MQTT"
            subtitle="Bridge weather updates through the Meshtastic-compatible MQTT feed."
            publisher={publishers.mqtt}
            updatePublisher={updatePublisher}
          />
        ) : null}
      </Stack>
    </SectionCard>
  );
}
