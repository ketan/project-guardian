import type { ReactNode } from "react";
import type { components } from "../api/generated";

export type DeviceConfig = components["schemas"]["DeviceConfigView"];
export type DeviceStatus = components["schemas"]["DeviceStatusResponse"];

export type UiPublisher = {
  id: string;
  type: "windy" | "wunderground" | "webhook" | "mqtt";
  enabled: boolean;
  publishIntervalSeconds: number;
  includeHistoryWindowMinutes?: number;
  stationId?: string;
  endpoint?: string;
  brokerUrl?: string;
  topic?: string;
  username?: string;
  apiKey?: string;
  password?: string;
  authHeader?: string;
  passwordConfigured?: boolean;
  apiKeyConfigured?: boolean;
  authHeaderConfigured?: boolean;
};

export type UiConfig = Omit<DeviceConfig, "publishers" | "network"> & {
  network: Omit<DeviceConfig["network"], "wifi" | "cellular"> & {
    wifi: DeviceConfig["network"]["wifi"] & {
      password?: string;
    };
    cellular: DeviceConfig["network"]["cellular"] & {
      pin?: string;
    };
  };
  publishers: UiPublisher[];
};

export type NavItem = {
  id: string;
  label: string;
  icon: ReactNode;
};
