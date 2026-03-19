import { DeviceStatusSchema, type DeviceStatus } from "./contracts";
import { STATUS_PATH } from "./paths";
import { requestJson } from "./request";
import type { ApiConnectionSettings } from "./runtime";

export async function fetchStatus(settings: ApiConnectionSettings): Promise<DeviceStatus> {
  return requestJson(STATUS_PATH, settings, { method: "GET" }, DeviceStatusSchema);
}
