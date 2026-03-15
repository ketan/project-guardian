# Weather Station Platform Restructure and v1 Architecture

## Summary
Build a two-part project with `backend/` for ESP32-S3 firmware and `frontend/` for a Vite + React admin app, with a top-level `openapi.yaml` as the contract between them. The firmware will sample the DFRobot `SEN0658` every 30 seconds, smooth selected measurements, log records to microSD as JSON, and publish to Weather Underground, Windy, and a generic webhook on independent configurable intervals. The system will be designed around measurement-type abstractions so future pressure/temperature/etc. hardware can be swapped without changing the rest of the stack.

Production access will target a direct HTTPS endpoint on the device when reachable, with GitHub Pages hosting the frontend shell and a planned relay fallback because carrier inbound reachability is still unknown. SMS admin will use a command-based interface, including `OPEN SESAME`, and the reply will include both a URL and raw IP details.

## Key Changes

### Project structure
- Keep firmware under `backend/` as the PlatformIO project.
- Create `frontend/` as a Vite + React app that consumes the OpenAPI contract.
- Add top-level `openapi.yaml` defining config, status, sensor readings, publisher settings, auth, and admin actions shared by backend and frontend.

### Firmware architecture
- Replace the single-file sketch with subsystems for boot/orchestration, config loading, sensor capability interfaces, storage/logging, smoothing, connectivity, publisher adapters, web API/auth, and SMS command handling.
- Model sensors around measurement capabilities rather than whole devices.
- Define interfaces such as `TemperatureSensor`, `PressureSensor`, `HumiditySensor`, `WindSensor`, `RainSensor`, `AirQualitySensor`, `LightSensor`, and `NoiseSensor`.
- Allow one composite hardware driver to implement multiple capabilities.
- Implement `SEN0658` as an RS485/Modbus composite device providing all supported measurements.
- Add a `Publisher` interface with concrete adapters for `wunderground`, `windy`, and `webhook`.
- Add MQTT publishing support as a Meshtastic-compatible protobuf publisher.
- Give each publisher its own config block, enable flag, credentials, and publish interval.
- Use a normalized internal weather sample model that publishers transform into service-specific payloads.
- Add a `CellularModem` abstraction with an initial `SIM7670G` implementation supporting power control, SMS read/delete, IP query, and data session lifecycle.

### Sampling, smoothing, sleep, and storage
- Use a 30-second sampling cadence as the base loop.
- On each wake: load config, read sensors, apply smoothing, append a timestamped JSON record to SD, evaluate publisher schedules, publish if due, and return to sleep unless in an admin-online window.
- Implement configurable smoothing with rolling average and/or EMA, with per-measurement enable/disable controls.
- Store runtime config primarily as JSON on microSD, with flash-backed fallback defaults for recovery if SD is missing or invalid.
- Retain the last 14 days of logged weather data on microSD for local viewing through the web UI, with automatic pruning of older records.
- Treat SD as the primary place for config, logs, and data; frontend assets can be added there later if direct on-device hosting becomes necessary.
- For JSON config and data responses, stream the files from storage as-is instead of loading large documents fully into memory first.
- For OTA updates, accept a firmware binary plus checksum over the API, write the upload to a temporary file on microSD, verify the checksum, move the verified image to a canonical OTA staging location on microSD, and reboot. If checksum verification fails, delete the uploaded file and return an error.

### Web API, auth, and frontend
- Define OpenAPI endpoints by category:
  - Auth:
    - `POST /api/v1/auth/login`
    - `POST /api/v1/auth/logout`
  - Status:
    - `GET /api/v1/status`
  - Config:
    - `GET /api/v1/config`
    - `PUT /api/v1/config/station`
    - `PUT /api/v1/config/sampling`
    - `PUT /api/v1/config/smoothing`
    - `PUT /api/v1/config/storage`
    - `PUT /api/v1/config/network`
    - `PUT /api/v1/config/sms-admin`
    - `PUT /api/v1/config/web-ui`
    - `PUT /api/v1/config/sensors`
    - `PUT /api/v1/config/publishers`
  - Sensors and logs:
    - `GET /api/v1/sensors/latest`
    - `GET /api/v1/logs/recent`
    - `GET /api/v1/logs/history`
  - Admin:
    - `POST /api/v1/admin/connectivity/test`
    - `POST /api/v1/admin/publishers/test`
    - `POST /api/v1/admin/ota`
- Use login plus short-lived bearer tokens for API auth.
- Expose CORS configuration for the production GitHub Pages origin and localhost dev origins.
- Build the React app around login, system status, sensor/smoothing config, publisher config, connectivity/modem status, historical charts for the last 14 days, and SMS/admin diagnostics.
- Keep config, log, and history access simple for device memory constraints: the config, recent-log, and history endpoints should stream the JSON documents currently stored by the ESP as-is, without server-side filtering, slicing, aggregation, or query parameters.
- Split config updates into small section-specific endpoints so the ESP only parses one subsection at a time instead of a large full-config payload.
- Keep the API contract compatible with both direct device access and a future relay/proxy without changing frontend behavior.

### SMS and remote admin flow
- Support command-based admin SMS with a parser and help system.
- Support a whitelist of trusted phone numbers allowed to issue SMS admin commands.
- Reject privileged SMS commands from non-whitelisted numbers.
- v1 commands should include `OPEN SESAME`, `STATUS`, `HELP`, `CONFIG GET <path>`, `CONFIG SET <path> <value>`, `NET`, and `PUBLISH NOW`.
- Successful `OPEN SESAME` handling from a whitelisted number should power up the modem, enter a configurable 5-15 minute always-awake admin window, bring up data, query IPv4/IPv6, and send an SMS reply containing both URL and IP details.
- If direct public access is unavailable, keep hooks for a future relay URL in the same reply format.
- During the admin window, suspend normal deep sleep and keep the API responsive; resume normal sampling/deep-sleep behavior after the window expires.

## Public Interfaces and Contract Additions
- `openapi.yaml` becomes the contract source for config schema, sensor status/read models, historical log document models, auth models, publisher settings, Meshtastic MQTT settings, OTA upload models, modem/network status, admin actions, and error responses.
- SMS configuration should include whitelisted administrator phone numbers and command authorization settings.
- Core backend types should include `WeatherSample`, `SmoothedSample`, `DeviceConfig`, `SensorConfig`, `PublisherConfig`, `ConnectivityConfig`, `SmsCommandResult`, and `AdminWindowState`.
- Config JSON should be versioned with a top-level schema version and a migration hook for future compatibility.

## Test Plan
- Unit tests for config parsing/validation, smoothing algorithms, publish scheduling, SMS command parsing/auth including whitelist handling, modem abstraction behavior, Meshtastic protobuf encoding, OTA checksum verification, and sensor capability aggregation.
- Integration-style tests for `SEN0658` sample acquisition with mocked Modbus responses, SD log append/readback, 14-day retention/pruning, publisher payload generation including Meshtastic MQTT protobuf messages, OTA staging file handling on microSD, and admin window state transitions.
- Integration-style tests should verify streamed JSON responses for config, recent logs, and history without requiring full-file buffering in memory.
- Integration-style tests should verify section-specific config updates so each endpoint can patch stored config safely without parsing a large full-config payload.
- API contract checks to validate backend responses against `openapi.yaml` and verify/generated frontend client bindings.
- Frontend tests for login, config editing, dashboard rendering, publisher forms, and API error handling.
- Field acceptance scenarios for normal boot, recovery boot, 30-second logging, scheduled publishing, `OPEN SESAME` SMS wake-up flow, and admin window expiry.

## Assumptions and Defaults
- Board remains `esp32-s3-devkitc-1` under `backend/`.
- `SEN0658` is treated as an RS485/Modbus composite sensor source.
- Runtime config is JSON-based, stored on SD first, with flash fallback.
- Weather service v1 targets are Weather Underground, Windy, generic webhook, and Meshtastic-compatible MQTT.
- Frontend production host is GitHub Pages; localhost is allowed for development.
- API auth uses bearer tokens.
- SMS admin uses command + help semantics, not conversational SMS sessions, and privileged access is granted only to whitelisted numbers.
- Carrier inbound reachability is unknown, so the implementation should preserve a relay fallback even though the preferred path is direct HTTPS on the ESP32.
- TLS is assumed to terminate on the ESP32 for the primary path, but the API/frontend design should not depend on that remaining the only deployment model.
