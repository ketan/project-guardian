# Weather Capture and Delivery Design

This is the target capture and delivery design. SD is the source of truth: append the record before any future network delivery, then publish that exact record. Failed delivery leaves the record queued; it is never reconstructed by rereading the sensor.

## Capture and aggregation

| Data | SEN0658 response time | Capture frequency | Record/publish cadence | Record value |
| --- | --- | --- | --- | --- |
| Wind speed and direction | 1 s | 1 Hz | Every 5 minutes | Two-minute vector-average sustained wind; highest three-second running-average gust from the last ten minutes |
| Temperature, humidity, noise | ≤1 s | 1/minute | Every 5 minutes | Mean of five readings |
| Atmospheric pressure, illuminance | ≤2 s | 1/minute | Every 5 minutes | Mean of five readings |
| PM2.5, PM10 | ≤90 s | 1/minute, as part of the complete sensor snapshot | Every 5 minutes | Latest reading; it is already sensor-smoothed |
| Rainfall | Not specified | 1/minute | Every 5 minutes | Rolling last-hour delta and current total from the cumulative counter; unavailable during the first hour after boot |

DFRobot requires host polling and response waits above 200 ms; response time is not a recommended host polling interval. See the [SEN0658 specification](https://wiki.dfrobot.com/sen0658/) for the response-time limits.

The wind ring contains 600 fixed four-byte samples (2,400 bytes) in ordinary RAM. Each sample stores centimetres per second and centi-degrees, with no heap allocation. Convert sensor values to those units at the read boundary. Do not persist the 1 Hz wind data.

For sustained wind, convert each speed/direction sample to north/east components, average the latest 120 samples, then use `atan2` to derive direction. This avoids the 359°/1° averaging error. Gust is the maximum of each three-sample speed average across the preceding ten-minute ring, using a `uint32_t` temporary sum.

Read the complete sensor snapshot once per minute. Keep a five-minute rolling stable-weather ring in RAM for temperature, humidity, pressure, illuminance, and noise; retain the latest snapshot for PM. Every five minutes, combine those values with the wind aggregate into one timestamped NDJSON record, append/close it on SD, and later publish that exact record.

Rainfall has a different measurement window from the five-minute upload cadence. Keep a separate 61-entry rainfall-counter ring sampled once per minute. Each five-minute record sends `rainfallLastHourMm = latest cumulative counter - counter from 60 minutes ago`, so records overlap: a 12:00 upload covers 11:00–12:00 and a 12:05 upload covers 11:05–12:05. Do not sum rainfall values across uploads. `rainfallTotalMm` preserves the raw cumulative counter. Until the ring contains a full hour after boot, `rainfallLastHourMm` is unavailable; do not report it as zero.

## Class boundaries

- `SEN0658` remains the Modbus driver. `readWind()` serves the 1 Hz wind path and `readLatestSnapshot()` serves the one-minute complete snapshot path.
- `TenMinuteWindCapture` owns only its 600 `WindSample` entries and calculates sustained wind and gust.
- `FiveMinuteWeatherCapture` owns the fixed rolling readings for the one-minute stable metrics and calculates their means. It does not own PM or rainfall history.
- `OneHourRainfallCapture` owns only 61 cumulative-rainfall values and calculates the last-hour delta.
- `WeatherRecord` is the one compact, timestamped record written to NDJSON and later published unchanged. Build it at the five-minute boundary from the aggregates plus the latest snapshot's PM reading.

Do not add an interface hierarchy or a scheduler class yet. The loop can call these four concrete components directly.

## Power and delivery

Poll wind at 1 Hz with timer-driven ESP32 light sleep between polls. Do not deep-sleep every second: it reboots the ESP and loses continuous capture state. Keep capture state in ordinary RAM.

LTE/Windy scheduling is not part of the capture path yet. When added, publish after the SD append and track delivery separately. Deployed LTE-only operation should disable Wi-Fi, Bluetooth, mDNS, captive portal, and Telnet; use SIM7670G PSM between uploads when supported and measure it against power-cycling the modem.

If a future deep-sleep design retains state, retain only compact aggregates with `RTC_DATA_ATTR`, a magic/version field, and validation. It is not durable across power loss.
