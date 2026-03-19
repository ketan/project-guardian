import { useCallback, useEffect, useState } from "react";
import type { DeviceStatus } from "../api/contracts";
import { hasApiConnectionSettings, type ApiConnectionSettings } from "../api/runtime";
import { fetchStatus } from "../api/statusApi";

export function useDeviceStatus(connection: ApiConnectionSettings) {
  const [status, setStatus] = useState<DeviceStatus | null>(null);
  const [isLoading, setIsLoading] = useState(false);
  const [loadError, setLoadError] = useState<string | null>(null);
  const canConnect = hasApiConnectionSettings(connection);

  const reload = useCallback(async () => {
    if (!canConnect) {
      setStatus(null);
      setLoadError(null);
      setIsLoading(false);
      return;
    }

    setIsLoading(true);
    setLoadError(null);

    try {
      const nextStatus = await fetchStatus(connection);
      setStatus(nextStatus);
      return nextStatus;
    } catch (error) {
      setLoadError(error instanceof Error ? error.message : "Failed to load device status");
      throw error;
    } finally {
      setIsLoading(false);
    }
  }, [canConnect, connection]);

  useEffect(() => {
    void reload().catch(() => undefined);
  }, [reload]);

  return { status, isLoading, loadError, reload, canConnect };
}
