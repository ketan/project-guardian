import { useEffect, useState } from "react";
import { zodResolver } from "@hookform/resolvers/zod";
import {
  Alert,
  AppShell,
  Badge,
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
import { useForm, type DefaultValues } from "react-hook-form";
import {
  IconAlertCircle,
  IconCloud,
  IconCpu,
  IconDeviceFloppy,
  IconFilter,
  IconMapPin,
  IconMessageCircle,
  IconRefresh,
  IconSend,
  IconWifi,
  IconClockHour4,
} from "@tabler/icons-react";
import { UiConfigSchema, type PublisherSlotKey, type UiConfig } from "./api/contracts";
import type { DeviceConfig } from "./types/ui";
import { NavigationPanel } from "./components/NavigationPanel";
import { SectionCard } from "./components/SectionCard";
import { SummaryCards } from "./components/SummaryCards";
import { BackendConnectionSection } from "./components/sections/BackendConnectionSection";
import { HealthSection } from "./components/sections/HealthSection";
import { NetworkSection } from "./components/sections/NetworkSection";
import { PublishersSection } from "./components/sections/PublishersSection";
import { SamplingSection } from "./components/sections/SamplingSection";
import { SensorsSection } from "./components/sections/SensorsSection";
import { SmoothingSection } from "./components/sections/SmoothingSection";
import { SmsAdminSection } from "./components/sections/SmsAdminSection";
import { StationSection } from "./components/sections/StationSection";
import { navItems } from "./data/mockDevice";
import { useApiConnectionSettings } from "./hooks/useApiConnectionSettings";
import { useDeviceConfig, useSequentialConfigSave } from "./hooks/useDeviceConfig";
import { useDeviceStatus } from "./hooks/useDeviceStatus";

function deepEqual(left: unknown, right: unknown) {
  return JSON.stringify(left) === JSON.stringify(right);
}

function getActiveTransportLabel(status: NonNullable<ReturnType<typeof useDeviceStatus>["status"]>) {
  const active: string[] = [];

  if (status.connectivity.wifi.active) {
    active.push("Wi-Fi");
  }

  if (status.connectivity.cellular.active) {
    active.push("Cellular");
  }

  if (active.length === 0) {
    return "No transport active";
  }

  return active.join(" + ");
}

type ConfigBlockKey =
  | "station"
  | "sampling"
  | "smoothing"
  | "sensors"
  | "network"
  | "publishers"
  | "smsAdmin";

function LoadingSection({
  id,
  title,
  subtitle,
  icon,
}: {
  id: string;
  title: string;
  subtitle: string;
  icon: React.ReactNode;
}) {
  return (
    <SectionCard id={id} title={title} subtitle={subtitle} icon={icon} loading>
      <div style={{ minHeight: 160 }} />
    </SectionCard>
  );
}

function App() {
  const [opened, { toggle, close }] = useDisclosure(false);
  const [validationError, setValidationError] = useState<string | null>(null);
  const { settings: apiSettings, normalizedBaseUrl, updateApiKey, updateBaseUrl } =
    useApiConnectionSettings();
  const { config: loadedConfig, isLoading, loadedSections, loadError, reload, canConnect } =
    useDeviceConfig(apiSettings);
  const {
    status,
    isLoading: isStatusLoading,
    loadError: statusLoadError,
    reload: reloadStatus,
  } = useDeviceStatus(apiSettings);
  const { save, isSaving, saveError, savingSection, lastSavedAt } =
    useSequentialConfigSave(apiSettings);

  const {
    watch,
    reset,
    getValues,
    setValue,
    handleSubmit,
    formState: { dirtyFields, isDirty },
  } = useForm<UiConfig>({
    resolver: zodResolver(UiConfigSchema) as never,
    defaultValues: { schemaVersion: 1 } as DefaultValues<UiConfig>,
    mode: "onBlur",
  });

  useEffect(() => {
    if (loadedConfig) {
      reset(loadedConfig as UiConfig, { keepDirtyValues: true });
    }
  }, [loadedConfig, reset]);

  const formConfig = watch();
  const config = formConfig as UiConfig;

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

  const updatePublisher = (slot: PublisherSlotKey, updates: Record<string, unknown>) => {
    const current = getValues(`publishers.${slot}` as never) as Record<string, unknown> | undefined;

    if (!current) {
      return;
    }

    setValue(`publishers.${slot}` as never, { ...current, ...updates } as never, {
      shouldDirty: true,
      shouldValidate: true,
    });
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

  const isBlockLoading = (block: ConfigBlockKey) => {
    if (!canConnect || loadError) {
      return false;
    }

    switch (block) {
      case "station":
        return !loadedSections.has("station");
      case "sampling":
        return !loadedSections.has("sampling") || !loadedSections.has("storage");
      case "smoothing":
        return !loadedSections.has("smoothing");
      case "sensors":
        return !loadedSections.has("sensors");
      case "network":
        return !loadedSections.has("network") || !loadedSections.has("webUi");
      case "publishers":
        return (
          !loadedSections.has("publishers.wunderground") ||
          !loadedSections.has("publishers.windy") ||
          !loadedSections.has("publishers.mqtt")
        );
      case "smsAdmin":
        return !loadedSections.has("smsAdmin");
    }
  };

  const isBlockLoaded = (block: ConfigBlockKey) => !isBlockLoading(block);

  const hasBlockData = (block: ConfigBlockKey) => {
    switch (block) {
      case "station":
        return Boolean(config.station);
      case "sampling":
        return Boolean(config.sampling && config.storage);
      case "smoothing":
        return Boolean(config.smoothing);
      case "sensors":
        return Array.isArray(config.sensors);
      case "network":
        return Boolean(config.network && config.webUi);
      case "publishers":
        return Boolean(
          config.publishers?.wunderground &&
            config.publishers?.windy &&
            config.publishers?.mqtt,
        );
      case "smsAdmin":
        return Boolean(config.smsAdmin);
    }
  };

  return (
    <AppShell
      header={{ height: { base: 104, sm: 72 } }}
      navbar={{ width: 320, breakpoint: "md", collapsed: { mobile: !opened, desktop: false } }}
      padding="xs"
    >
      <AppShell.Header>
        <Group h="100%" px="md" justify="space-between">
          <Group gap="sm" style={{ flex: 1, minWidth: 0 }} wrap="nowrap">
            <Burger opened={opened} onClick={toggle} hiddenFrom="md" size="sm" />
            <Stack gap={0} style={{ minWidth: 0 }}>
              <Title order={4}>Weather station admin</Title>
              <Text c="dimmed" size="sm" style={{ whiteSpace: "normal" }}>
                Controls for field weather station health, publishers, and pilot-facing weather ops.
              </Text>
            </Stack>
          </Group>

          <Group gap="sm" wrap="nowrap">
            <Badge
              leftSection={<IconCloud size={14} stroke={1.75} />}
              color="green"
              variant="light"
              radius="sm"
              visibleFrom="sm"
            >
              {status
                ? getActiveTransportLabel(status)
                : isStatusLoading
                  ? "Loading status"
                  : "Status unavailable"}
            </Badge>
            <Button
              leftSection={
                isSaving ? <Loader color="currentColor" size={14} /> : <IconDeviceFloppy size={16} stroke={1.75} />
              }
              color="teal"
              disabled={isLoading || !canConnect}
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
              <Title order={1}>Weather control panel</Title>
              <Text c="dimmed" maw={860}>
                Tune the station, review device health, and publish reliable local conditions that
                pilots can compare against forecast models over the last few days.
              </Text>
            </Stack>

            {!canConnect ? (
              <Alert color="blue" title="Backend connection required" variant="light">
                <Text size="sm">
                  Enter both the backend API base URL and API key below to load and save device configuration.
                </Text>
              </Alert>
            ) : null}

            {loadError ? (
              <Alert
                color="red"
                icon={<IconAlertCircle size={18} stroke={1.75} />}
                title="Could not load configuration"
                variant="light"
              >
                <Group justify="space-between" wrap="nowrap">
                  <Text size="sm">{loadError}</Text>
                  <Button leftSection={<IconRefresh size={16} stroke={1.75} />} size="compact-sm" variant="light" onClick={() => void reload()}>
                    Retry
                  </Button>
                </Group>
              </Alert>
            ) : null}

            {statusLoadError ? (
              <Alert
                color="yellow"
                icon={<IconAlertCircle size={18} stroke={1.75} />}
                title="Could not load device status"
                variant="light"
              >
                <Group justify="space-between" wrap="nowrap">
                  <Text size="sm">{statusLoadError}</Text>
                  <Button
                    leftSection={<IconRefresh size={16} stroke={1.75} />}
                    size="compact-sm"
                    variant="light"
                    onClick={() => void reloadStatus()}
                  >
                    Retry
                  </Button>
                </Group>
              </Alert>
            ) : null}

            {saveError ? (
              <Alert color="red" icon={<IconAlertCircle size={18} stroke={1.75} />} title="Save failed" variant="light">
                <Text size="sm">{saveError}</Text>
              </Alert>
            ) : null}

            {validationError ? (
              <Alert color="yellow" icon={<IconAlertCircle size={18} stroke={1.75} />} title="Validation needed" variant="light">
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

            <Stack gap="lg">
              <BackendConnectionSection
                settings={apiSettings}
                normalizedBaseUrl={normalizedBaseUrl}
                updateApiKey={updateApiKey}
                updateBaseUrl={updateBaseUrl}
              />

              {loadError ? null : (
                <>
                  {status ? <SummaryCards status={status} /> : null}

                  {isBlockLoaded("station") && hasBlockData("station") ? (
                    <StationSection
                      config={config}
                      loading={false}
                      updateStationField={updateStationField}
                      setConfig={applyConfigUpdate}
                    />
                  ) : canConnect ? (
                    <LoadingSection
                      id="station"
                      title="Station identity"
                      subtitle="Core site metadata for public weather services and forecast comparison."
                      icon={<IconMapPin size={18} stroke={1.75} />}
                    />
                  ) : null}

                  <SimpleGrid cols={1} spacing="lg" verticalSpacing="lg">
                    <Stack gap="lg">
                      {isBlockLoaded("sampling") && hasBlockData("sampling") ? (
                        <SamplingSection
                          config={config}
                          loading={false}
                          updateSamplingField={updateSamplingField}
                          setConfig={applyConfigUpdate}
                        />
                      ) : canConnect ? (
                        <LoadingSection
                          id="sampling"
                          title="Sampling and retention"
                          subtitle="Compact controls for timing, sleep, and on-device history."
                          icon={<IconClockHour4 size={18} stroke={1.75} />}
                        />
                      ) : null}

                      {isBlockLoaded("smoothing") && hasBlockData("smoothing") ? (
                        <SmoothingSection
                          config={config}
                          loading={false}
                          setConfig={applyConfigUpdate}
                        />
                      ) : canConnect ? (
                        <LoadingSection
                          id="smoothing"
                          title="Smoothing"
                          subtitle="Tune which measurements get averaged before charting and publishing."
                          icon={<IconFilter size={18} stroke={1.75} />}
                        />
                      ) : null}

                      {isBlockLoaded("sensors") && hasBlockData("sensors") ? (
                        <SensorsSection
                          config={config}
                          loading={false}
                          setConfig={applyConfigUpdate}
                        />
                      ) : canConnect ? (
                        <LoadingSection
                          id="sensors"
                          title="Sensors"
                          subtitle="Configure attached sensor modules and their connection settings."
                          icon={<IconCpu size={18} stroke={1.75} />}
                        />
                      ) : null}

                      {isBlockLoaded("network") && hasBlockData("network") ? (
                        <NetworkSection
                          config={config}
                          loading={false}
                          setConfig={applyConfigUpdate}
                          wifiConnected={status?.connectivity.wifi.connected ?? false}
                          signalQuality={status?.connectivity.cellular.signalQuality}
                        />
                      ) : canConnect ? (
                        <LoadingSection
                          id="network"
                          title="Connectivity"
                          subtitle="Prefer the best transport automatically, but keep Wi-Fi and LTE settings easy to reach from a phone."
                          icon={<IconWifi size={18} stroke={1.75} />}
                        />
                      ) : null}
                    </Stack>

                    <Stack gap="lg">
                      {isBlockLoaded("publishers") && hasBlockData("publishers") ? (
                        <PublishersSection
                          loading={false}
                          publishers={config.publishers}
                          updatePublisher={updatePublisher}
                        />
                      ) : canConnect ? (
                        <LoadingSection
                          id="publishers"
                          title="Publishers"
                          subtitle="Configure where the station pushes weather updates and how often each destination is used."
                          icon={<IconSend size={18} stroke={1.75} />}
                        />
                      ) : null}

                      {isBlockLoaded("smsAdmin") && hasBlockData("smsAdmin") ? (
                        <SmsAdminSection
                          config={config}
                          loading={false}
                          setConfig={applyConfigUpdate}
                        />
                      ) : canConnect ? (
                        <LoadingSection
                          id="sms"
                          title="SMS administration"
                          subtitle="Trusted numbers can wake the station and manage connectivity without a password."
                          icon={<IconMessageCircle size={18} stroke={1.75} />}
                        />
                      ) : null}

                      {status ? <HealthSection status={status} /> : null}
                    </Stack>
                  </SimpleGrid>
                </>
              )}
            </Stack>
          </Stack>
        </Container>
      </AppShell.Main>
    </AppShell>
  );
}

export default App;
