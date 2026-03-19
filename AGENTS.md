# Shared Project Rules

- Treat `openapi.yaml` as the API contract source of truth.
- If API behavior or endpoints change, update `openapi.yaml` and `docs/PLAN.md` in the same change.
- Keep frontend and backend request/response shapes aligned with `openapi.yaml`.
- Prefer simple, explicit code over indirection unless the abstraction clearly improves readability.
- Validate data at the boundary before mutating backend state or using API payloads in the frontend.
