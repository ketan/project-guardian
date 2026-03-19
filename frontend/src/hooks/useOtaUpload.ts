import { useState } from "react";
import { uploadOtaFirmware } from "../api/otaApi";
import { hasApiConnectionSettings, type ApiConnectionSettings } from "../api/runtime";
import type { OtaUploadResult } from "../api/contracts";

export function useOtaUpload(connection: ApiConnectionSettings) {
  const [isUploading, setIsUploading] = useState(false);
  const [uploadError, setUploadError] = useState<string | null>(null);
  const [result, setResult] = useState<OtaUploadResult | null>(null);
  const canConnect = hasApiConnectionSettings(connection);

  async function upload(firmware: File, checksum: string) {
    if (!canConnect) {
      const message = "Enter both the backend API base URL and API key before uploading firmware.";
      setUploadError(message);
      throw new Error(message);
    }

    setIsUploading(true);
    setUploadError(null);
    setResult(null);

    try {
      const nextResult = await uploadOtaFirmware(connection, firmware, checksum);
      setResult(nextResult);
      return nextResult;
    } catch (error) {
      const message = error instanceof Error ? error.message : "OTA upload failed";
      setUploadError(message);
      throw error;
    } finally {
      setIsUploading(false);
    }
  }

  function clearResult() {
    setResult(null);
    setUploadError(null);
  }

  return { upload, isUploading, uploadError, result, clearResult, canConnect };
}
