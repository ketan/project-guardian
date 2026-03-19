import { OtaUploadResponseSchema, type OtaUploadResult } from "./contracts";
import { OTA_UPLOAD_PATH } from "./paths";
import { requestFormData } from "./request";
import type { ApiConnectionSettings } from "./runtime";

export async function uploadOtaFirmware(
  settings: ApiConnectionSettings,
  firmware: File,
  sha256sum: string,
): Promise<OtaUploadResult> {
  const formData = new FormData();
  formData.set("firmware", firmware);
  formData.set("sha256sum", sha256sum.trim());

  return requestFormData(OTA_UPLOAD_PATH, settings, formData, OtaUploadResponseSchema);
}
