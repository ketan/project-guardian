#include "MultiWiFi.h"

#include <Logger.h>
#include <uri/UriGlob.h>

bool MultiWiFi::begin(const char *portalSsid, const char *portalPassword) {
    preferences.begin("wifi-networks", false);
    WiFi.mode(WIFI_STA);
    loadNetworks();

    if (preferences.getUChar("count", 0) > 0 && networks.run(10000) == WL_CONNECTED) {
        INFO("Wi-Fi connected: %s", WiFi.SSID().c_str());
        return true;
    }

    startPortal(portalSsid, portalPassword);
    return false;
}

void MultiWiFi::loop() {
    if (!portalActive) {
        return;
    }
    dns.processNextRequest();
    portal.handleClient();
}

bool MultiWiFi::isPortalActive() const {
    return portalActive;
}

void MultiWiFi::loadNetworks() {
    const uint8_t count = preferences.getUChar("count", 0);
    for (uint8_t index = 0; index < count && index < maxNetworks; ++index) {
        const String ssid = preferences.getString(key("ssid", index).c_str(), "");
        const String password = preferences.getString(key("pass", index).c_str(), "");
        if (!ssid.isEmpty()) {
            networks.addAP(ssid.c_str(), password.c_str());
        }
    }
}

void MultiWiFi::startPortal(const char *ssid, const char *password) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid, password);
    dns.start(53, "*", WiFi.softAPIP());

    portal.on("/", HTTP_GET, [this]() {
        sendPortalPage();
    });
    portal.on("/", HTTP_POST, [this]() {
        const String ssid = portal.arg("ssid");
        const String password = portal.arg("password");
        if (ssid.isEmpty()) {
            portal.send(400, "text/plain", "Network is required.");
            return;
        }
        saveNetwork(ssid, password);
        portal.send(200, "text/html", "Saved. Restarting...");
        delay(250);
        ESP.restart();
    });
    portal.on("/generate_204", HTTP_GET, [this]() { portal.sendHeader("Location", "/"); portal.send(302); });
    portal.on(UriGlob("/*"), HTTP_ANY, [this]() { portal.sendHeader("Location", "/"); portal.send(302); });
    portal.begin();
    portalActive = true;
    INFO("Wi-Fi setup portal active: %s", ssid);
}

void MultiWiFi::sendPortalPage() {
    String page = "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
                  "<title>Project Guardian Wi-Fi</title><style>"
                  "*{box-sizing:border-box}body{margin:0;min-height:100vh;display:grid;place-items:center;background:#f2f5f3;color:#18231d;font:16px system-ui,sans-serif}"
                  "main{width:min(100%,34rem);margin:1rem;padding:1.5rem;background:white;border-radius:1rem;box-shadow:0 8px 28px #0002}"
                  "h1{margin:0 0:.25rem}p{color:#526158}table{width:100%;border-collapse:collapse;margin:1rem 0}th,td{padding:.75rem .5rem;border-bottom:1px solid #dfe6e1;text-align:left}"
                  "th:first-child,td:first-child{width:3rem;text-align:center}input[type=radio]{width:1.2rem;height:1.2rem;accent-color:#176b3a}"
                  "input[type=password]{width:100%;padding:.7rem;border:1px solid #aebbb2;border-radius:.4rem;font:inherit}button{width:100%;padding:.8rem;border:0;border-radius:.4rem;background:#176b3a;color:white;font:inherit;font-weight:600}"
                  "</style><main><h1>Connect Wi-Fi</h1><p>Choose a nearby network, enter its password, then save.</p>"
                  "<form method=post><table><thead><tr><th>Select</th><th>Network</th><th>Signal</th></tr></thead><tbody>";
    constexpr uint8_t maxDisplayedNetworks = 16;
    const int networkCount = WiFi.scanNetworks();
    for (int index = 0; index < networkCount && index < maxDisplayedNetworks; ++index) {
        const String ssid = WiFi.SSID(index);
        if (ssid.isEmpty()) {
            continue;
        }
        char rssi[8];
        snprintf(rssi, sizeof(rssi), "%d", WiFi.RSSI(index));
        page += "<tr><td><input type=radio name=ssid value=\"";
        page += escapeHtml(ssid);
        page += "\" required></td><td>";
        page += escapeHtml(ssid);
        page += "</td><td>";
        page += rssi;
        page += " dBm</td></tr>";
    }
    WiFi.scanDelete();
    page += "</tbody></table><p><label>Password<br><input name=password type=password autocomplete=current-password></label></p>"
            "<p><button>Save and restart</button></p></form></main>";
    portal.send(200, "text/html", page);
}

void MultiWiFi::saveNetwork(const String &ssid, const String &password) {
    uint8_t count = preferences.getUChar("count", 0);
    for (uint8_t index = 0; index < count; ++index) {
        if (preferences.getString(key("ssid", index).c_str(), "") == ssid) {
            preferences.putString(key("pass", index).c_str(), password);
            return;
        }
    }

    const uint8_t index = count < maxNetworks ? count : maxNetworks - 1;
    preferences.putString(key("ssid", index).c_str(), ssid);
    preferences.putString(key("pass", index).c_str(), password);
    if (count < maxNetworks) {
        preferences.putUChar("count", count + 1);
    }
}

String MultiWiFi::key(const char *prefix, uint8_t index) {
    char value[12];
    snprintf(value, sizeof(value), "%s%u", prefix, index);
    return value;
}

String MultiWiFi::escapeHtml(const String &value) {
    String escaped;
    for (const char *character = value.c_str(); *character != '\0'; ++character) {
        switch (*character) {
            case '&': escaped += "&amp;"; break;
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&#39;"; break;
            default: escaped += *character; break;
        }
    }
    return escaped;
}
