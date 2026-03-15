import { useEffect, useState } from "react";
import { zodResolver } from "@hookform/resolvers/zod";
import {
  Alert,
  AppShell,
  Badge,
  Box,
  Burger,
  Button,
  Container,
  Group,
  Loader,
  SimpleGrid,
  Stack,
  Text,
  Title,
} from "@mantine/core";
import { useDisclosure } from "@mantine/hooks";
import { useForm } from "react-hook-form";
import { FiAlertCircle, FiCloud, FiRefreshCw, FiSave } from "react-icons/fi";
import { UiConfigSchema, type UiConfig, type UiPublisher } from "./api/contracts";
import type { DeviceConfig } from "./types/ui";
import { NavigationPanel } from "./components/NavigationPanel";
import { SummaryCards } from "./components/SummaryCards";
import { HealthSection } from "./components/sections/HealthSection";
import { NetworkSection } from "./components/sections/NetworkSection";
import { PublishersSection } from "./components/sections/PublishersSection";
import { SamplingSection } from "./components/sections/SamplingSection";
import { SensorsSection } from "./components/sections/SensorsSection";
import { SmoothingSection } from "./components/sections/SmoothingSection";
import { SmsAdminSection } from "./components/sections/SmsAdminSection";
import { StationSection } from "./components/sections/StationSection";
import { initialConfig, initialStatus, navItems } from "./data/mockDevice";
import { useDeviceConfig, useSequentialConfigSave } from "./hooks/useDeviceConfig";

function deepEqual(left: unknown, right: unknown) {
  return JSON.stringify(left) === JSON.stringify(right);
}

function App() {
  const [opened, { toggle, close }] = useDisclosure(false);
  const [validationError, setValidationError] = useState<string | null>(null);
  const status = initialStatus;
  const { config: loadedConfig, isLoading, loadError, reload } = useDeviceConfig();
  const { save, isSaving, saveError, savingSection, lastSavedAt } = useSequentialConfigSave();

  const {
    watch,
    reset,
    getValues,
    setValue,
    handleSubmit,
    formState: { dirtyFields, isDirty },
  } = useForm<UiConfig>({
    resolver: zodResolver(UiConfigSchema) as never,
    defaultValues: initialConfig,
    mode: "onBlur",
  });

  useEffect(() => {
    if (loadedConfig) {
      reset(loadedConfig);
    }
  }, [loadedConfig, reset]);

  const config = watch();

  const applyConfigUpdate: React.Dispatch<React.SetStateAction<UiConfig>> = (updater) => {
    const current = getValues();
    const next = typeof updater === "function" ? updater(current) : updater;

    (Object.keys(next) as (keyof UiConfig)[]).forEach((key) => {
      if (!deepEqual(current[key], next[key])) {
        setValue(key, next[key], { shouldDirty: true, shouldValidate: true });
      }
    });
  };

  const updateStationField = <K extends keyof DeviceConfig["station"]>(
    key: K,
    value: DeviceConfig["station"][K],
  ) => {
    setValue(`station.${String(key)}` as never, value, {
      shouldDirty: true,
      shouldValidate: true,
    });
  };

  const updateSamplingField = <K extends keyof DeviceConfig["sampling"]>(
    key: K,
    value: DeviceConfig["sampling"][K],
  ) => {
    setValue(`sampling.${String(key)}` as never, value, {
      shouldDirty: true,
      shouldValidate: true,
    });
  };

  const updatePublisher = (publisherId: string, updates: Partial<UiPublisher>) => {
    const nextPublishers = getValues("publishers").map((publisher: UiPublisher) =>
      publisher.id === publisherId ? { ...publisher, ...updates } : publisher,
    ) as UiPublisher[];

    setValue("publishers", nextPublishers, { shouldDirty: true, shouldValidate: true });
  };

  const onSubmit = handleSubmit(async (values: UiConfig) => {
    setValidationError(null);
    const { savedSections } = await save(values, dirtyFields);

    if (savedSections.length === 0) {
      return;
    }

    try {
      const refreshed = await reload();
      void refreshed;
    } finally {
      reset(values);
    }
  }, () => {
    setValidationError("Please fix the highlighted configuration values before saving.");
  });

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
            <Button
              leftSection={isSaving ? <Loader color="currentColor" size={14} /> : <FiSave />}
              color="teal"
              disabled={isLoading}
              loading={isSaving}
              onClick={() => void onSubmit()}
            >
              {isSaving ? `Saving ${savingSection ?? "config"}...` : isDirty ? "Save changes" : "Saved"}
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

            {loadError ? (
              <Alert
                color="red"
                icon={<FiAlertCircle />}
                title="Could not load configuration"
                variant="light"
              >
                <Group justify="space-between" wrap="nowrap">
                  <Text size="sm">{loadError}</Text>
                  <Button leftSection={<FiRefreshCw />} size="compact-sm" variant="light" onClick={() => void reload()}>
                    Retry
                  </Button>
                </Group>
              </Alert>
            ) : null}

            {saveError ? (
              <Alert color="red" icon={<FiAlertCircle />} title="Save failed" variant="light">
                <Text size="sm">{saveError}</Text>
              </Alert>
            ) : null}

            {validationError ? (
              <Alert color="yellow" icon={<FiAlertCircle />} title="Validation needed" variant="light">
                <Text size="sm">{validationError}</Text>
              </Alert>
            ) : null}

            {lastSavedAt ? (
              <Alert color="teal" title="Configuration saved" variant="light">
                <Text size="sm">
                  Saved at {lastSavedAt.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" })}.
                </Text>
              </Alert>
            ) : null}

            <Box pos="relative">
              {isLoading ? (
                <Group justify="center" py="xl">
                  <Loader size="lg" />
                </Group>
              ) : (
                loadError || !loadedConfig ? null : (
                  <>
                  <SummaryCards status={status} />

                  <StationSection
                    config={config}
                    updateStationField={updateStationField}
                    setConfig={applyConfigUpdate}
                  />

                  <SimpleGrid cols={{ base: 1, lg: 2 }} spacing="lg" verticalSpacing="lg">
                    <Stack gap="lg">
                      <SamplingSection
                        config={config}
                        updateSamplingField={updateSamplingField}
                        setConfig={applyConfigUpdate}
                      />
                      <SmoothingSection config={config} setConfig={applyConfigUpdate} />
                      <SensorsSection config={config} setConfig={applyConfigUpdate} />
                      <NetworkSection
                        config={config}
                        setConfig={applyConfigUpdate}
                        wifiConnected={status.connectivity.wifi.connected}
                        signalQuality={status.connectivity.cellular.signalQuality}
                      />
                    </Stack>

                    <Stack gap="lg">
                      <PublishersSection
                        publishers={config.publishers}
                        updatePublisher={updatePublisher}
                      />
                      <SmsAdminSection config={config} setConfig={applyConfigUpdate} />
                      <HealthSection status={status} />
                    </Stack>
                  </SimpleGrid>
                  </>
                )
              )}
            </Box>
          </Stack>
        </Container>
      </AppShell.Main>
    </AppShell>
  );
}

export default App;
