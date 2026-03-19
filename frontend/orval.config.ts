import {defineConfig} from "orval";

export default defineConfig({
  guardian: {
    input: {
      target: "../openapi.yaml",
    },
    output: {
      client: "zod",
      clean: true,
      target: "src/api/generated/orval.zod.ts",
      schemas: {
        path: "src/api/generated/schemas",
        type: "zod",
      },
      override: {
        aliasCombinedTypes: true,
        zod: {
          dateTimeOptions: {
            offset: true,
          },
        }
      },
    },
  },
});
