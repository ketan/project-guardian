import { Badge, Grid, PasswordInput, Stack, Text, TextInput } from "@mantine/core";
import { SectionCard } from "../SectionCard";
import type { ApiConnectionSettings } from "../../api/runtime";

type BackendConnectionSectionProps = {
  settings: ApiConnectionSettings;
  normalizedBaseUrl: string;
  updateBaseUrl: (baseUrl: string) => void;
  updateApiKey: (apiKey: string) => void;
};

export function BackendConnectionSection({
  settings,
  normalizedBaseUrl,
  updateBaseUrl,
  updateApiKey,
}: BackendConnectionSectionProps) {
  return (
    <SectionCard
      id="backend-connection"
      title="Backend connection"
      subtitle="Point the admin UI at a device hostname or IP and provide the bearer token used for authenticated API requests."
      action={
        normalizedBaseUrl ? (
          <Badge color="teal" radius="sm" variant="light">
            {normalizedBaseUrl}
          </Badge>
        ) : null
      }
    >
      <Grid>
        <Grid.Col span={12}>
          <TextInput
            label="Backend API base URL"
            placeholder="weather-station.local:8080"
            value={settings.baseUrl}
            onChange={(event) => updateBaseUrl(event.currentTarget.value)}
          />
        </Grid.Col>
        <Grid.Col span={12}>
          <PasswordInput
            label="Bearer token"
            placeholder="Required for authenticated API calls"
            value={settings.apiKey}
            onChange={(event) => updateApiKey(event.currentTarget.value)}
          />
        </Grid.Col>
      </Grid>

      <Stack gap={2}>
        <Text c="dimmed" size="sm">
          If you enter only a hostname or IP, the UI assumes `http://` automatically.
        </Text>
        <Text c="dimmed" size="sm">
          The entered token is sent as the `Authorization: Bearer ...` header on every `/api` request.
        </Text>
      </Stack>
    </SectionCard>
  );
}
