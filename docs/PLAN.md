# Weather Station Platform Restructure and v1 Architecture

## Summary
Build a two-part project with `backend/` for ESP32-S3 firmware and `frontend/` for a Vite + React admin app, with a top-level `openapi.yaml` as the contract between them. The firmware will sample the DFRobot `SEN0658` every 30 seconds, smooth selected measurements, log records to microSD as JSON, and publish to Weather Underground, Windy, and Meshtastic-compatible MQTT on independent configurable intervals. The system will be designed around measurement-type abstractions so future pressure/temperature/etc. hardware can be swapped without changing the rest of the stack.

Production access will target a direct HTTPS endpoint on the device when reachable, with GitHub Pages hosting the frontend shell and a planned relay fallback because carrier inbound reachability is still unknown. SMS admin will use a command-based interface, including `OPEN SESAME`, and the reply will include both a URL and raw IP details.

## Current Status

### Completed
- [x] Split the project into `backend/` and `frontend/`.
- [x] Define a top-level `openapi.yaml` contract for the current device API.
- [x] Simplify the config API to section-specific `GET` and `PUT` endpoints.
- [x] Simplify publisher configuration to named slots for `wunderground`, `windy`, and `mqtt`.
- [x] Build a mobile-friendly frontend with Vite+, React, Mantine, and Tabler icons.
- [x] Add backend connection settings in the frontend for base URL plus bearer token.
- [x] Implement frontend config loading with generated Zod validation.
- [x] Implement sequential per-section config loads so the UI does not overwhelm the ESP32.
- [x] Implement sequential dirty-only config saves from the UI.
- [x] Implement real status loading in the frontend from `GET /api/v1/status`.
- [x] Add OTA upload to the API contract and frontend UI.
- [x] Add frontend tests for config validation, dirty-save behavior, sequencing, retry flows, and status/config error handling.

### Pending
- [ ] Implement the backend HTTP server in `backend/`.
- [ ] Implement handwritten C++ config/status models and JSON parsing/serialization on the firmware side.
- [ ] Implement sensor abstraction and the `SEN0658` integration.
- [ ] Implement persistence to microSD, including streaming JSON responses directly from storage.
- [ ] Implement 14-day retention and history generation on the device.
- [ ] Implement publisher runtime behavior for Weather Underground, Windy, and Meshtastic MQTT.
- [ ] Implement modem, SMS command handling, and admin window behavior.
- [ ] Implement OTA staging behavior on the device side, including temp-file write, verification, staging, and reboot.
- [ ] Add a frontend view for latest live sensor readings from `GET /api/v1/sensors/latest`.
- [ ] Add frontend history visualization for the retained weather log from `GET /api/v1/logs/history`.
- [ ] Add frontend OTA tests covering upload success and failure behavior.

## Key Changes

### Project structure
- Keep firmware under `backend/` as the PlatformIO project.
- Create `frontend/` as a Vite+ React app that consumes the OpenAPI contract.
- Add top-level `openapi.yaml` defining config, status, sensor readings, publisher settings, auth, and admin actions shared by backend and frontend.

### Firmware architecture
- Replace the single-file sketch with subsystems for boot/orchestration, config loading, sensor capability interfaces, storage/logging, smoothing, connectivity, publisher adapters, web API/auth, and SMS command handling.
- Model sensors around measurement capabilities rather than whole devices.
- Define interfaces such as `TemperatureSensor`, `PressureSensor`, `HumiditySensor`, `WindSensor`, `RainSensor`, `AirQualitySensor`, `LightSensor`, and `NoiseSensor`.
- Allow one composite hardware driver to implement multiple capabilities.
- Implement `SEN0658` as an RS485/Modbus composite device providing all supported measurements.
- Add a `Publisher` interface with concrete adapters for `wunderground`, `windy`, and Meshtastic-compatible `mqtt`.
- Add MQTT publishing support as a Meshtastic-compatible protobuf publisher.
- Give each publisher its own config block, enable flag, credentials, and publish interval.
- Use a normalized internal weather sample model that publishers transform into service-specific payloads.
- Add a `CellularModem` abstraction with an initial `SIM7670G` implementation supporting power control, SMS read/delete, IP query, and data session lifecycle.

### Sampling, smoothing, sleep, and storage
- Use a 30-second sampling cadence as the base loop.
- On each wake: load config, read sensors, apply smoothing, append a timestamped JSON record to SD, evaluate publisher schedules, publish if due, and return to sleep unless in an admin-online window.
- If GPS-based location is enabled and GPS hardware is available, use the most recent GPS fix for station latitude/longitude/elevation instead of manual coordinates.
- Poll GPS infrequently to save power, using a configurable refresh interval of every few hours and caching the last valid fix between GPS polls.
- Implement configurable smoothing with rolling average and/or EMA, with per-measurement enable/disable controls.
- Store runtime config primarily as JSON on microSD, with flash-backed fallback defaults for recovery if SD is missing or invalid.
- Retain the last 14 days of logged weather data on microSD for local viewing through the web UI, with automatic pruning of older records.
- Treat SD as the primary place for config, logs, and data; frontend assets can be added there later if direct on-device hosting becomes necessary.
- For JSON config and data responses, stream the files from storage as-is instead of loading large documents fully into memory first.
- For OTA updates, accept a firmware binary plus checksum over the API, write the upload to a temporary file on microSD, verify the checksum, move the verified image to a canonical OTA staging location on microSD, and reboot. If checksum verification fails, delete the uploaded file and return an error.

### Web API, auth, and frontend
- Define OpenAPI endpoints by category:
  - Status:
    - `GET /api/v1/status`
  - Config:
    - `GET /api/v1/config/station`
    - `PUT /api/v1/config/station`
    - `GET /api/v1/config/sampling`
    - `PUT /api/v1/config/sampling`
    - `GET /api/v1/config/smoothing`
    - `PUT /api/v1/config/smoothing`
    - `GET /api/v1/config/storage`
    - `PUT /api/v1/config/storage`
    - `GET /api/v1/config/network`
    - `PUT /api/v1/config/network`
    - `GET /api/v1/config/sms-admin`
    - `PUT /api/v1/config/sms-admin`
    - `GET /api/v1/config/web-ui`
    - `PUT /api/v1/config/web-ui`
    - `GET /api/v1/config/sensors`
    - `PUT /api/v1/config/sensors`
    - `GET /api/v1/config/publishers/wunderground`
    - `PUT /api/v1/config/publishers/wunderground`
    - `GET /api/v1/config/publishers/windy`
    - `PUT /api/v1/config/publishers/windy`
    - `GET /api/v1/config/publishers/mqtt`
    - `PUT /api/v1/config/publishers/mqtt`
  - Sensors and logs:
    - `GET /api/v1/sensors/latest`
    - `GET /api/v1/logs/history`
  - Admin:
    - `POST /api/v1/admin/ota`
- Use a bearer token supplied by the admin UI for API auth.
- Expose CORS configuration for the production GitHub Pages origin and localhost dev origins.
- Build the React app around backend connection settings, system status, live/latest sensor readings, sensor/smoothing config, publisher config, connectivity/modem status, OTA upload, historical charts for the last 14 days, and SMS/admin diagnostics.
- Keep log and history access simple for device memory constraints: the history endpoint should stream the JSON document currently stored by the ESP as-is, without server-side filtering, slicing, aggregation, or query parameters.
- Split both config reads and config updates into small section-specific endpoints so the ESP only parses one subsection at a time instead of a large full-config payload.
- Load and save config sections sequentially from the UI so the ESP32 handles one request at a time.
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
- `openapi.yaml` becomes the contract source for config schema, sensor status/read models, historical log document models, publisher settings, Meshtastic MQTT settings, OTA upload models, modem/network status, GPS location settings, and error responses.
- SMS configuration should include whitelisted administrator phone numbers and command authorization settings.
- Core backend types should include `WeatherSample`, `SmoothedSample`, `DeviceConfig`, `SensorConfig`, `PublisherConfig`, `ConnectivityConfig`, `GpsLocationState`, `SmsCommandResult`, and `AdminWindowState`.
- Config JSON should be versioned with a top-level schema version and a migration hook for future compatibility.

## Test Plan
- Unit tests for config parsing/validation, smoothing algorithms, publish scheduling, GPS poll scheduling and fallback behavior, SMS command parsing/auth including whitelist handling, modem abstraction behavior, Meshtastic protobuf encoding, OTA checksum verification, and sensor capability aggregation.
- Integration-style tests for `SEN0658` sample acquisition with mocked Modbus responses, SD log append/readback, 14-day retention/pruning, publisher payload generation including Meshtastic MQTT protobuf messages, OTA staging file handling on microSD, and admin window state transitions.
- Integration-style tests should verify streamed JSON responses for history without requiring full-file buffering in memory.
- Integration-style tests should verify section-specific config updates so each endpoint can patch stored config safely without parsing a large full-config payload.
- API contract checks to validate backend responses against `openapi.yaml` and verify/generated frontend Zod schemas.
- Frontend tests for config editing, dashboard rendering, publisher forms, OTA upload, sequential load/save behavior, and API error handling.
- Field acceptance scenarios for normal boot, recovery boot, 30-second logging, scheduled publishing, `OPEN SESAME` SMS wake-up flow, and admin window expiry.

## Assumptions and Defaults
- Board remains `esp32-s3-devkitc-1` under `backend/`.
- `SEN0658` is treated as an RS485/Modbus composite sensor source.
- Runtime config is JSON-based, stored on SD first, with flash fallback.
- Weather service v1 targets are Weather Underground, Windy, and Meshtastic-compatible MQTT.
- Frontend production host is GitHub Pages; localhost is allowed for development.
- API auth uses a bearer token configured in the admin UI.
- SMS admin uses command + help semantics, not conversational SMS sessions, and privileged access is granted only to whitelisted numbers.
- Carrier inbound reachability is unknown, so the implementation should preserve a relay fallback even though the preferred path is direct HTTPS on the ESP32.
- TLS is assumed to terminate on the ESP32 for the primary path, but the API/frontend design should not depend on that remaining the only deployment model.
