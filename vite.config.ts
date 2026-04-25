import { defineConfig } from "vite";
import react from "@vitejs/plugin-react-swc";
import { VitePWA } from "vite-plugin-pwa";
import path from "path";
import { componentTagger } from "lovable-tagger";

export default defineConfig(({ mode }) => ({
  server: {
    host: true,
    port: 8080
  },

  plugins: [
    react(),

    VitePWA({
      registerType: "autoUpdate",
      manifest: {
        name: "Smart Clock Manager",
        short_name: "SmartClock",
        description: "Manage tasks for ESP32 Smart Clock",
        theme_color: "#000000",
        background_color: "#ffffff",
        display: "standalone",
        start_url: "/",

        icons: [
          {
            src: "/icon-192.png",
            sizes: "192x192",
            type: "image/png"
          },
          {
            src: "/icon-512.png",
            sizes: "512x512",
            type: "image/png"
          }
        ]
      }
    }),

    mode === "development" && componentTagger()
  ].filter(Boolean),

  resolve: {
    alias: {
      "@": path.resolve(__dirname, "./src")
    }
  }
}));