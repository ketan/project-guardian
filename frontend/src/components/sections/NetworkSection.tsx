import { Badge, Grid, Group, Select, TagsInput, Text, TextInput } from "@mantine/core";
import { FiShield, FiSmartphone, FiWifi } from "react-icons/fi";
import { SectionCard } from "../SectionCard";
import type { UiConfig } from "../../types/ui";

type NetworkSectionProps = {
  config: UiConfig;
  setConfig: React.Dispatch<React.SetStateAction<UiConfig>>;
  wifiConnected: boolean;
  signalQuality?: number;
};

export function NetworkSection({
  config,
  setConfig,
  wifiConnected,
  signalQuality,
}: NetworkSectionProps) {
  return (
    <SectionCard
      id="network"
      title="Connectivity"
      subtitle="Prefer the best transport automatically, but keep Wi-Fi and LTE settings easy to reach from a phone."
    >
      <Grid>
        <Grid.Col span={{ base: 12, sm: 6 }}>
          <Select
            label="Preferred transport"
            data={[
              { value: "auto", label: "Auto" },
              { value: "wifi", label: "Wi-Fi" },
              { value: "cellular", label: "Cellular" },
            ]}
            value={config.network.preferredTransport}
            onChange={(value) =>
              value
                ? setConfig((current) => ({
                    ...current,
                    network: {
                      ...current.network,
                      preferredTransport: value as UiConfig["network"]["preferredTransport"],
                    },
                  }))
                : undefined
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 6 }}>
          <TextInput
            label="Wi-Fi SSID"
            value={config.network.wifi.ssid}
            onChange={(event) =>
              setConfig((current) => ({
                ...current,
                network: {
                  ...current.network,
                  wifi: { ...current.network.wifi, ssid: event.currentTarget.value },
                },
              }))
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 6 }}>
          <TextInput
            label="Cellular APN"
            value={config.network.cellular.apn ?? ""}
            onChange={(event) =>
              setConfig((current) => ({
                ...current,
                network: {
                  ...current.network,
                  cellular: { ...current.network.cellular, apn: event.currentTarget.value },
                },
              }))
            }
          />
        </Grid.Col>
        <Grid.Col span={{ base: 12, sm: 6 }}>
          <TagsInput
            label="Allowed origins"
            value={config.webUi.allowedOrigins}
            onChange={(value) =>
              setConfig((current) => ({
                ...current,
                webUi: { ...current.webUi, allowedOrigins: value },
              }))
            }
          />
        </Grid.Col>
      </Grid>

      <Group gap="sm" mt="sm">
        <Badge leftSection={<FiWifi />} variant="light" radius="sm">
          Wi-Fi {wifiConnected ? "connected" : "offline"}
        </Badge>
        <Badge leftSection={<FiSmartphone />} variant="light" radius="sm" color="orange">
          LTE SQI {signalQuality ?? 0}
        </Badge>
        <Badge leftSection={<FiShield />} variant="light" radius="sm" color="grape">
          Token TTL {config.webUi.tokenTtlMinutes} min
        </Badge>
      </Group>

      <Text c="dimmed" size="sm">
        GitHub Pages and localhost are treated as allowed origins for the admin UI, while the
        device still keeps data delivery simple by streaming stored JSON files directly.
      </Text>
    </SectionCard>
  );
}
