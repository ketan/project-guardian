import { useState } from "react";
import {
  Alert,
  Button,
  FileInput,
  Loader,
  Stack,
  Text,
} from "@mantine/core";
import { IconAlertCircle, IconBinaryTree2, IconUpload, IconCircleCheck } from "@tabler/icons-react";
import { SectionCard } from "../SectionCard";
import { useOtaUpload } from "../../hooks/useOtaUpload";
import type { ApiConnectionSettings } from "../../api/runtime";

type OtaSectionProps = {
  connection: ApiConnectionSettings;
};

async function calculateSha256(file: File): Promise<string> {
  const buffer = await file.arrayBuffer();
  const digest = await crypto.subtle.digest("SHA-256", buffer);
  return Array.from(new Uint8Array(digest))
    .map((byte) => byte.toString(16).padStart(2, "0"))
    .join("");
}

export function OtaSection({ connection }: OtaSectionProps) {
  const [firmware, setFirmware] = useState<File | null>(null);
  const [sha256sum, setSha256sum] = useState("");
  const [isHashing, setIsHashing] = useState(false);
  const [hashError, setHashError] = useState<string | null>(null);
  const { upload, isUploading, uploadError, result, clearResult, canConnect } = useOtaUpload(connection);

  async function handleFileChange(file: File | null) {
    setFirmware(file);
    setHashError(null);
    clearResult();

    if (!file) {
      setSha256sum("");
      return;
    }

    if (typeof crypto === "undefined" || !crypto.subtle) {
      setHashError("Browser SHA-256 support is unavailable. Enter the checksum manually.");
      return;
    }

    setIsHashing(true);

    try {
      const nextChecksum = await calculateSha256(file);
      setSha256sum(nextChecksum);
    } catch {
      setHashError("Could not calculate the firmware SHA-256 checksum automatically.");
    } finally {
      setIsHashing(false);
    }
  }

  async function handleUpload() {
    if (!firmware || !sha256sum.trim()) {
      return;
    }

    try {
      await upload(firmware, sha256sum);
    } catch {
      return;
    }
  }

  return (
    <SectionCard
      id="ota"
      title="Firmware update"
      subtitle="Upload a firmware image and stage it for OTA installation after the device verifies it."
      icon={<IconBinaryTree2 size={18} stroke={1.75} />}
    >
      <Stack gap="md">
        {uploadError ? (
          <Alert color="red" icon={<IconAlertCircle size={18} stroke={1.75} />} title="OTA upload failed" variant="light">
            <Text size="sm">{uploadError}</Text>
          </Alert>
        ) : null}

        {hashError ? (
          <Alert color="yellow" icon={<IconAlertCircle size={18} stroke={1.75} />} title="Checksum warning" variant="light">
            <Text size="sm">{hashError}</Text>
          </Alert>
        ) : null}

        {result ? (
          <Alert color="teal" icon={<IconCircleCheck size={18} stroke={1.75} />} title="Firmware staged" variant="light">
            <Stack gap={2}>
              <Text size="sm">{result.message}</Text>
              {typeof result.firmwareSizeBytes === "number" ? (
                <Text c="dimmed" size="sm">{`Size: ${result.firmwareSizeBytes.toLocaleString()} bytes`}</Text>
              ) : null}
              {result.stagedPath ? (
                <Text c="dimmed" size="sm">{`Staged at: ${result.stagedPath}`}</Text>
              ) : null}
            </Stack>
          </Alert>
        ) : null}

        <FileInput
          label="Firmware binary"
          placeholder="Choose a .bin file"
          accept=".bin,application/octet-stream"
          value={firmware}
          onChange={(file) => void handleFileChange(file)}
        />

        {isHashing ? (
          <Text c="dimmed" size="sm">
            Preparing firmware package...
          </Text>
        ) : null}

        <Stack gap={4}>
          <Text c="dimmed" size="sm">
            The device will verify the uploaded firmware before staging it for installation.
          </Text>
          <Button
            leftSection={isUploading ? <Loader size={14} color="currentColor" /> : <IconUpload size={16} stroke={1.75} />}
            loading={isUploading}
            disabled={!canConnect || !firmware || !sha256sum.trim() || isHashing}
            onClick={() => void handleUpload()}
          >
            {isUploading ? "Uploading..." : "Upload firmware"}
          </Button>
        </Stack>
      </Stack>
    </SectionCard>
  );
}
