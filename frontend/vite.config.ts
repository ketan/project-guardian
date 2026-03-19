import {defineConfig} from "vite-plus";
import react from "@vitejs/plugin-react";

const PROXY_DELAY_MS = 200;

// https://vite.dev/config/
export default defineConfig({
  lint: {options: {typeAware: true, typeCheck: true}},
  plugins: [react()],
  test: {
    environment: "jsdom",
    setupFiles: "./src/test/setup.ts",
    globals: true,
  },
  server: {
    proxy: {
      "/api": {
        target: "http://localhost:8080",
        changeOrigin: true,
        rewrite: (path) => {
          sleep(PROXY_DELAY_MS);
          return path
        }
      },
    },
  },
});

function sleep(delay: number) {
  const start = new Date().getTime()
  while (new Date().getTime() - start < delay) {
    /* empty */
  }
}
