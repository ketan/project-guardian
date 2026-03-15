import { defineConfig } from "orval";

export default defineConfig({
  guardian: {
    input: {
      target: "../openapi.yaml",
    },
    output: {
      mode: "single",
      client: "fetch",
      target: "src/api/generated/client.ts",
      schemas: {
        path: "src/api/generated/schemas",
        type: "zod",
      },
      override: {
        aliasCombinedTypes: true,
        fetch: {
          includeHttpResponseReturnType: false,
          runtimeValidation: true,
        },
      },
    },
  },
});
