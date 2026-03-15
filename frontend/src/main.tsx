import React from "react";
import ReactDOM from "react-dom/client";
import { MantineProvider, createTheme } from "@mantine/core";
import "@mantine/core/styles.css";
import "./index.css";
import App from "./App";

const theme = createTheme({
  primaryColor: "teal",
  fontFamily: '"IBM Plex Sans", "Segoe UI", sans-serif',
  defaultRadius: "md",
  headings: {
    fontFamily: '"IBM Plex Sans", "Segoe UI", sans-serif',
  },
});

ReactDOM.createRoot(document.getElementById("root")!).render(
  <React.StrictMode>
    <MantineProvider theme={theme}>
      <App />
    </MantineProvider>
  </React.StrictMode>,
);
