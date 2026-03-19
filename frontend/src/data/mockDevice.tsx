import {
  IconClockHour4,
  IconCpu,
  IconFilter,
  IconHeartbeat,
  IconMapPin,
  IconMessageCircle,
  IconBinaryTree2,
  IconSend,
  IconServer,
  IconWifi,
} from "@tabler/icons-react";
import type {NavItem} from "../types/ui";

export const navItems: NavItem[] = [
  { id: "backend-connection", label: "Backend connection", icon: <IconServer size={18} stroke={1.75} /> },
  { id: "station", label: "Station identity", icon: <IconMapPin size={18} stroke={1.75} /> },
  { id: "sampling", label: "Sampling and retention", icon: <IconClockHour4 size={18} stroke={1.75} /> },
  { id: "smoothing", label: "Smoothing", icon: <IconFilter size={18} stroke={1.75} /> },
  { id: "sensors", label: "Sensors", icon: <IconCpu size={18} stroke={1.75} /> },
  { id: "network", label: "Connectivity", icon: <IconWifi size={18} stroke={1.75} /> },
  { id: "publishers", label: "Publishers", icon: <IconSend size={18} stroke={1.75} /> },
  { id: "sms", label: "SMS administration", icon: <IconMessageCircle size={18} stroke={1.75} /> },
  { id: "overview-health", label: "Sensor and storage health", icon: <IconHeartbeat size={18} stroke={1.75} /> },
  { id: "ota", label: "Firmware update", icon: <IconBinaryTree2 size={18} stroke={1.75} /> },
];
