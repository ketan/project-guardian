import {
  ActionIcon,
  Badge,
  Button,
  Card,
  Grid,
  Group,
  Stack,
  Switch,
  TextInput,
} from "@mantine/core";
import { FiPlus, FiTrash2 } from "react-icons/fi";
import { SectionCard } from "../SectionCard";
import type { UiConfig } from "../../types/ui";

type SmsAdminSectionProps = {
  config: UiConfig;
  setConfig: React.Dispatch<React.SetStateAction<UiConfig>>;
};

const MAX_SMS_ADMINS = 5;

export function SmsAdminSection({ config, setConfig }: SmsAdminSectionProps) {
  const addWhitelistEntry = () => {
    if (config.smsAdmin.whitelist.length >= MAX_SMS_ADMINS) {
      return;
    }

    setConfig((current: UiConfig) => ({
      ...current,
      smsAdmin: {
        ...current.smsAdmin,
        whitelist: [
          ...current.smsAdmin.whitelist,
          { label: "New contact", phoneNumber: "+91" },
        ],
      },
    }));
  };

  const updateWhitelistEntry = (
    index: number,
    updates: Partial<UiConfig["smsAdmin"]["whitelist"][number]>,
  ) => {
    setConfig((current: UiConfig) => ({
      ...current,
      smsAdmin: {
        ...current.smsAdmin,
        whitelist: current.smsAdmin.whitelist.map((entry: UiConfig["smsAdmin"]["whitelist"][number], entryIndex: number) =>
          entryIndex === index ? { ...entry, ...updates } : entry,
        ),
      },
    }));
  };

  const removeWhitelistEntry = (index: number) => {
    setConfig((current: UiConfig) => ({
      ...current,
      smsAdmin: {
        ...current.smsAdmin,
        whitelist: current.smsAdmin.whitelist.filter((_: UiConfig["smsAdmin"]["whitelist"][number], entryIndex: number) => entryIndex !== index),
      },
    }));
  };

  return (
    <SectionCard
      id="sms"
      title="SMS administration"
      subtitle="Trusted numbers can wake the station and manage connectivity without a password."
      action={
        <Button
          leftSection={<FiPlus />}
          size="compact-sm"
          variant="light"
          onClick={addWhitelistEntry}
          disabled={config.smsAdmin.whitelist.length >= MAX_SMS_ADMINS}
        >
          Add number
        </Button>
      }
    >
      <Stack gap="md">
        <Switch
          checked={config.smsAdmin.enabled}
          onChange={(event) =>
            setConfig((current: UiConfig) => ({
              ...current,
              smsAdmin: { ...current.smsAdmin, enabled: event.currentTarget.checked },
            }))
          }
          label="SMS admin enabled"
        />

        <Stack gap="sm">
          <Badge variant="light" radius="sm">
            {config.smsAdmin.whitelist.length} / {MAX_SMS_ADMINS} trusted numbers
          </Badge>
          {config.smsAdmin.whitelist.map((entry: UiConfig["smsAdmin"]["whitelist"][number], index: number) => (
            <Card key={`${entry.phoneNumber}-${index}`} withBorder padding="md">
              <Stack gap="sm">
                <Group justify="space-between" align="center">
                  <Badge variant="light" radius="sm">
                    Trusted number
                  </Badge>
                  <ActionIcon
                    color="red"
                    variant="light"
                    onClick={() => removeWhitelistEntry(index)}
                    aria-label={`Remove ${entry.label}`}
                  >
                    <FiTrash2 />
                  </ActionIcon>
                </Group>

                <Grid>
                  <Grid.Col span={{ base: 12, sm: 5 }}>
                    <TextInput
                      label="Label"
                      value={entry.label}
                      onChange={(event) =>
                        updateWhitelistEntry(index, { label: event.currentTarget.value })
                      }
                    />
                  </Grid.Col>
                  <Grid.Col span={{ base: 12, sm: 7 }}>
                    <TextInput
                      label="Phone number"
                      value={entry.phoneNumber}
                      onChange={(event) =>
                        updateWhitelistEntry(index, { phoneNumber: event.currentTarget.value })
                      }
                    />
                  </Grid.Col>
                </Grid>
              </Stack>
            </Card>
          ))}
        </Stack>
      </Stack>
    </SectionCard>
  );
}
