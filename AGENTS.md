# Shared Project Rules

- This is a two-part weather station: ESP32-S3 firmware in `backend/` and the React admin UI in `frontend/`.
- Read the applicable subtree guide before changing either area:
  - [Firmware conventions](backend/AGENTS.md)
  - [Frontend conventions](frontend/AGENTS.md)
- Treat `openapi.yaml` as the API contract source of truth.
- If API behavior or endpoints change, update `openapi.yaml` and `docs/PLAN.md` in the same change.
- Keep frontend and backend request/response shapes aligned with `openapi.yaml`.
- After changing the OpenAPI contract, regenerate the frontend Zod schemas with `vp run generate:api` from `frontend/`; do not hand-edit `frontend/src/api/generated/`.
- Prefer simple, explicit code over indirection unless the abstraction clearly improves readability.
- Validate data at the boundary before mutating backend state or using API payloads in the frontend.
- The production sensor is a DFRobot SEN0658 connected over RS485/Modbus RTU through a 3.3 V TTL-to-RS485 adapter; preserve the distinction between the sensor's 10–30 V supply and ESP32 logic levels.
- The device is resource constrained: keep API/config operations section-sized and stream stored JSON rather than buffering large payloads.
- Do not add, log, or commit real Wi-Fi, bearer-token, SMS, MQTT, or weather-service credentials. Treat the current literal values in firmware as development placeholders to be replaced by configuration/storage work.
