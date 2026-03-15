import { useState } from "react";
import { AppShell, Badge, Burger, Button, Container, Group, SimpleGrid, Stack, Text, Title } from "@mantine/core";
import { useDisclosure } from "@mantine/hooks";
import { FiCloud, FiSave } from "react-icons/fi";
import { NavigationPanel } from "./components/NavigationPanel";
import { SummaryCards } from "./components/SummaryCards";
import { HealthSection } from "./components/sections/HealthSection";
import { NetworkSection } from "./components/sections/NetworkSection";
import { PublishersSection } from "./components/sections/PublishersSection";
import { SamplingSection } from "./components/sections/SamplingSection";
import { SmsAdminSection } from "./components/sections/SmsAdminSection";
import { StationSection } from "./components/sections/StationSection";
import { initialConfig, initialStatus, navItems } from "./data/mockDevice";
import type { DeviceConfig, UiConfig } from "./types/ui";

function App() {
  const [opened, { toggle, close }] = useDisclosure(false);
  const [config, setConfig] = useState<UiConfig>(initialConfig);
  const status = initialStatus;

  const updateStationField = <K extends keyof DeviceConfig["station"]>(
    key: K,
    value: DeviceConfig["station"][K],
  ) => {
    setConfig((current) => ({
      ...current,
      station: {
        ...current.station,
        [key]: value,
      },
    }));
  };

  const updateSamplingField = <K extends keyof DeviceConfig["sampling"]>(
    key: K,
    value: DeviceConfig["sampling"][K],
  ) => {
    setConfig((current) => ({
      ...current,
      sampling: {
        ...current.sampling,
        [key]: value,
      },
    }));
  };

  return (
    <AppShell
      header={{ height: 72 }}
      navbar={{ width: 320, breakpoint: "md", collapsed: { mobile: !opened, desktop: false } }}
      padding="md"
    >
      <AppShell.Header>
        <Group h="100%" px="md" justify="space-between">
          <Group gap="sm">
            <Burger opened={opened} onClick={toggle} hiddenFrom="md" size="sm" />
            <Stack gap={0}>
              <Title order={4}>Weather station admin</Title>
              <Text c="dimmed" size="sm" visibleFrom="sm">
                Mobile-friendly controls for a field weather station
              </Text>
            </Stack>
          </Group>

          <Group gap="sm">
            <Badge
              leftSection={<FiCloud size={12} />}
              color="green"
              variant="light"
              radius="sm"
              visibleFrom="sm"
            >
              {status.connectivity.activeTransport === "wifi" ? "Wi-Fi online" : "Cellular online"}
            </Badge>
            <Button leftSection={<FiSave />} color="teal">
              Save draft
            </Button>
          </Group>
        </Group>
      </AppShell.Header>

      <AppShell.Navbar>
        <NavigationPanel items={navItems} status={status} onNavigate={close} />
      </AppShell.Navbar>

      <AppShell.Main>
        <Container size="xl">
          <Stack gap="lg">
            <Stack gap="xs" id="overview">
              <Title order={1}>Flight-day weather control panel</Title>
              <Text c="dimmed" maw={860}>
                Tune the station, review device health, and publish reliable local conditions that
                pilots can compare against forecast models over the last few days.
              </Text>
            </Stack>

            <SummaryCards status={status} />

            <StationSection
              config={config}
              updateStationField={updateStationField}
              setConfig={setConfig}
            />

            <SimpleGrid cols={{ base: 1, lg: 2 }} spacing="lg" verticalSpacing="lg">
              <Stack gap="lg">
                <SamplingSection
                  config={config}
                  updateSamplingField={updateSamplingField}
                  setConfig={setConfig}
                />
                <NetworkSection
                  config={config}
                  setConfig={setConfig}
                  wifiConnected={status.connectivity.wifi.connected}
                  signalQuality={status.connectivity.cellular.signalQuality}
                />
              </Stack>

              <Stack gap="lg">
                <PublishersSection publishers={config.publishers} />
                <SmsAdminSection config={config} setConfig={setConfig} />
                <HealthSection status={status} />
              </Stack>
            </SimpleGrid>
          </Stack>
        </Container>
      </AppShell.Main>
    </AppShell>
  );
}

export default App;
