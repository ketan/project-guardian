import type { ReactNode } from "react";
export type { DeviceStatus, UiConfig, UiPublisher } from "../api/contracts";
import type { UiConfig } from "../api/contracts";

export type DeviceConfig = UiConfig;

export type NavItem = {
  id: string;
  label: string;
  icon: ReactNode;
};
