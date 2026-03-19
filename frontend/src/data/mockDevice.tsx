import {
  IconChartHistogram,
  IconCpu,
  IconFilter,
  IconMapPin,
  IconMessageCircle,
  IconSend,
  IconServer,
  IconWifi,
} from "@tabler/icons-react";
import type {NavItem} from "../types/ui";

export const navItems: NavItem[] = [
  { id: "overview", label: "Overview", icon: <IconChartHistogram size={18} stroke={1.75} /> },
  { id: "backend-connection", label: "Backend connection", icon: <IconServer size={18} stroke={1.75} /> },
  { id: "station", label: "Station identity", icon: <IconMapPin size={18} stroke={1.75} /> },
  { id: "sampling", label: "Sampling and retention", icon: <IconChartHistogram size={18} stroke={1.75} /> },
  { id: "smoothing", label: "Smoothing", icon: <IconFilter size={18} stroke={1.75} /> },
  { id: "sensors", label: "Sensors", icon: <IconCpu size={18} stroke={1.75} /> },
  { id: "network", label: "Connectivity", icon: <IconWifi size={18} stroke={1.75} /> },
  { id: "publishers", label: "Publishers", icon: <IconSend size={18} stroke={1.75} /> },
  { id: "sms", label: "SMS administration", icon: <IconMessageCircle size={18} stroke={1.75} /> },
];
