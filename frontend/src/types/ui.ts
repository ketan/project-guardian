import type { ReactNode } from "react";
import type { components } from "../api/generated";

export type DeviceConfig = components["schemas"]["DeviceConfigView"];
export type DeviceStatus = components["schemas"]["DeviceStatusResponse"];

export type UiPublisher = {
  id: string;
  type: "windy" | "wunderground" | "webhook" | "mqtt";
  enabled: boolean;
  publishIntervalSeconds: number;
  stationId?: string;
  brokerUrl?: string;
  topic?: string;
  username?: string;
  passwordConfigured?: boolean;
  apiKeyConfigured?: boolean;
};

export type UiConfig = Omit<DeviceConfig, "publishers"> & {
  publishers: UiPublisher[];
};

export type NavItem = {
  id: string;
  label: string;
  icon: ReactNode;
};

