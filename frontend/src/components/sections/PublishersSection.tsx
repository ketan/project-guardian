import { Badge, Card, Group, Stack, Text } from "@mantine/core";
import { SectionCard } from "../SectionCard";
import type { UiPublisher } from "../../types/ui";

type PublishersSectionProps = {
  publishers: UiPublisher[];
};

export function PublishersSection({ publishers }: PublishersSectionProps) {
  return (
    <SectionCard
      id="publishers"
      title="Publishers"
      subtitle="Keep outbound publishing simple and easy to verify in the field."
    >
      <Stack gap="md">
        {publishers.map((publisher) => (
          <Card key={publisher.id} withBorder padding="lg">
            <Stack gap="sm">
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
              <Text c="dimmed" size="sm">
                Publishes every {publisher.publishIntervalSeconds}s
              </Text>
              {publisher.stationId ? <Text size="sm">Station: {publisher.stationId}</Text> : null}
              {publisher.brokerUrl ? (
                <Text size="sm" style={{ overflowWrap: "anywhere" }}>
                  MQTT: {publisher.brokerUrl}
                </Text>
              ) : null}
            </Stack>
          </Card>
        ))}
      </Stack>
    </SectionCard>
  );
}
