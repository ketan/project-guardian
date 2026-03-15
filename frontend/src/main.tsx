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
  components: {
    TextInput: {
      defaultProps: {
        size: "xs",
      },
    },
    NumberInput: {
      defaultProps: {
        size: "xs",
      },
    },
    Textarea: {
      defaultProps: {
        size: "xs",
      },
    },
    Select: {
      defaultProps: {
        size: "xs",
      },
    },
    TagsInput: {
      defaultProps: {
        size: "xs",
      },
    },
    SegmentedControl: {
      defaultProps: {
        size: "xs",
      },
    },
  },
});

ReactDOM.createRoot(document.getElementById("root")!).render(
  <React.StrictMode>
    <MantineProvider theme={theme}>
      <App />
    </MantineProvider>
  </React.StrictMode>,
);
