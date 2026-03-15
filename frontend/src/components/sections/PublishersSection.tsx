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
import { SectionCard } from "../SectionCard";
import type { UiPublisher } from "../../types/ui";

type PublishersSectionProps = {
  publishers: UiPublisher[];
  updatePublisher: (publisherId: string, updates: Partial<UiPublisher>) => void;
};

export function PublishersSection({ publishers, updatePublisher }: PublishersSectionProps) {
  return (
    <SectionCard
      id="publishers"
      title="Publishers"
      subtitle="Configure where the station pushes weather updates and how often each destination is used."
    >
      <Stack gap="md">
        {publishers.map((publisher) => (
          <Card key={publisher.id} withBorder padding="lg">
            <Stack gap="md">
              <Group justify="space-between" align="flex-start">
                <Stack gap={0}>
                  <Text fw={700}>{publisher.id}</Text>
                  <Text size="sm" c="dimmed">
                    {publisher.type} publisher
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
                  updatePublisher(publisher.id, { enabled: event.currentTarget.checked })
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
                        ? updatePublisher(publisher.id, { publishIntervalSeconds: value })
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
                      updatePublisher(publisher.id, {
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
                        value={publisher.stationId ?? ""}
                        onChange={(event) =>
                          updatePublisher(publisher.id, { stationId: event.currentTarget.value })
                        }
                      />
                    </Grid.Col>
                    <Grid.Col span={{ base: 12, sm: 6 }}>
                      <PasswordInput
                        label="API key"
                        placeholder={publisher.apiKeyConfigured ? "Configured" : "Enter API key"}
                        value={publisher.apiKey ?? ""}
                        onChange={(event) =>
                          updatePublisher(publisher.id, { apiKey: event.currentTarget.value })
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
                        value={publisher.endpoint ?? ""}
                        onChange={(event) =>
                          updatePublisher(publisher.id, { endpoint: event.currentTarget.value })
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
                          updatePublisher(publisher.id, { authHeader: event.currentTarget.value })
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
                        value={publisher.brokerUrl ?? ""}
                        onChange={(event) =>
                          updatePublisher(publisher.id, { brokerUrl: event.currentTarget.value })
                        }
                      />
                    </Grid.Col>
                    <Grid.Col span={{ base: 12, sm: 6 }}>
                      <TextInput
                        label="Topic"
                        value={publisher.topic ?? ""}
                        onChange={(event) =>
                          updatePublisher(publisher.id, { topic: event.currentTarget.value })
                        }
                      />
                    </Grid.Col>
                    <Grid.Col span={{ base: 12, sm: 6 }}>
                      <TextInput
                        label="Username"
                        value={publisher.username ?? ""}
                        onChange={(event) =>
                          updatePublisher(publisher.id, { username: event.currentTarget.value })
                        }
                      />
                    </Grid.Col>
                    <Grid.Col span={12}>
                      <PasswordInput
                        label="Password"
                        placeholder={publisher.passwordConfigured ? "Configured" : "Enter password"}
                        value={publisher.password ?? ""}
                        onChange={(event) =>
                          updatePublisher(publisher.id, { password: event.currentTarget.value })
                        }
                      />
                    </Grid.Col>
                  </>
                )}
              </Grid>
            </Stack>
          </Card>
        ))}
      </Stack>
    </SectionCard>
  );
}
