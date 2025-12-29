#include "OTA.hpp"
#include "database.h"

// ===== Public =====
OTAProvisioner ota;

bool OTAProvisioner::beginAP(const char* ssid,
                             const char* password,
                             bool captivePortal,
                             IPAddress apIP,
                             IPAddress gateway,
                             IPAddress subnet)
{
  // Stop any STA mode and start AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, gateway, subnet);

  bool apOK = false;
  if (password && strlen(password) >= 8) {
    apOK = WiFi.softAP(ssid, password);
  } else {
    apOK = WiFi.softAP(ssid); // open AP if no/short password
  }
  if (!apOK) return false;

  // Optional mDNS for convenience (works for clients that support it)
  // esp32-ota.local → 192.168.4.1
  MDNS.begin("esp32-ota");
  MDNS.addService("http", "tcp", 80);

  // Captive portal (very lightweight)
  _useDNS = captivePortal;
  if (_useDNS) {
    // Resolve all requests to AP IP
    _dns.start(53, "*", apIP);
  }

  _registerRoutes();
  _server.begin();
  return true;
}

void OTAProvisioner::loop() {
  if (_useDNS) _dns.processNextRequest();
  _server.handleClient();
}

void OTAProvisioner::setBasicAuth(const char* user, const char* pass) {
  if (user && pass && strlen(user) && strlen(pass)) {
    _useAuth = true;
    _authUser = user;
    _authPass = pass;
  } else {
    _useAuth = false;
    _authUser = "";
    _authPass = "";
  }
}

// ===== Private =====

void OTAProvisioner::_registerRoutes() {
  using namespace std::placeholders;

  _server.on("/", HTTP_GET, std::bind(&OTAProvisioner::_handleRoot, this));
  // Upload form target
  _server.on(
    "/update",
    HTTP_POST,
    std::bind(&OTAProvisioner::_handleUpdate, this),         // called after upload completed
    std::bind(&OTAProvisioner::_handleUpload, this)          // handles the upload chunks
  );
  _server.on("/format", HTTP_GET, std::bind(&OTAProvisioner::_handleFormat, this));
  _server.onNotFound(std::bind(&OTAProvisioner::_handleNotFound, this));
}

bool OTAProvisioner::_checkAuth() {
  if (!_useAuth) return true;
  if (_server.authenticate(_authUser.c_str(), _authPass.c_str())) return true;
  _server.requestAuthentication();  // sends 401
  return false;
}

String OTAProvisioner::_buildPage(const String& msg) {
  String html =
    "<!doctype html><html><head><meta charset='utf-8'/>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'/>"
    "<title>ESP32 OTA Provisioning</title>"
    "<style>"
    "body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Helvetica,Arial,sans-serif;"
    "margin:0;padding:24px;background:#0b1020;color:#e8ecf1}"
    ".card{max-width:640px;margin:0 auto;background:#141a33;border-radius:16px;padding:24px;"
    "box-shadow:0 10px 30px rgba(0,0,0,.35)}"
    "h1{font-size:1.4rem;margin:0 0 12px}"
    "p{opacity:.8;margin:.5rem 0 1rem}"
    "form{margin-top:12px}"
    "input[type=file]{display:block;margin:12px 0;padding:8px;background:#0b1020;color:#e8ecf1;border:1px solid #2b3a76;border-radius:8px}"
    "button{padding:10px 16px;border:0;border-radius:10px;background:#3b82f6;color:#fff;font-weight:600;cursor:pointer}"
    ".msg{margin-top:12px;padding:10px;border-radius:8px;background:#0f172a;border:1px solid #334155}"
    ".ok{border-color:#22c55e}.err{border-color:#ef4444}"
    "code{background:#0b1020;padding:2px 6px;border-radius:6px;border:1px solid #2b3a76}"
    "</style></head><body><div class='card'>"
    "<h1>ESP32 OTA Provisioning</h1>"
    "<p>Connect to this hotspot and upload a compiled <code>.bin</code> firmware.</p>"
    "<form method='POST' action='/update' enctype='multipart/form-data'>"
    "<input type='file' name='firmware' accept='.bin' required/>"
    "<button type='submit'>Upload & Update</button>"
    "</form>";

  if (msg.length()) {
    html += "<div class='msg " + String(msg.startsWith("OK") ? "ok" : "err") + "'>";
    html += msg;
    html += "</div>";
  }

  html +=
    "<p style='opacity:.7;margin-top:16px'>AP IP: <code>" + WiFi.softAPIP().toString() +
    "</code> • mDNS: <code>esp32-ota.local</code></p>"
    "</div></body></html>";
  return html;
}

void OTAProvisioner::_handleRoot() {
  if (!_checkAuth()) return;

  // If client expected a captive portal redirect, serve the main page anyway
  if (_useDNS) {
    String host = _server.hostHeader();
    if (!host.equals(WiFi.softAPIP().toString()) && host != "esp32-ota.local") {
      // Basic captive portal redirect
      _server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
      _server.send(302, "text/plain", "");
      return;
    }
  }

  _server.send(200, "text/html", _buildPage());
}


void OTAProvisioner::_handleFormat() {
  if (!_checkAuth()) return;

  // If client expected a captive portal redirect, serve the main page anyway
  if (_useDNS) {
    String host = _server.hostHeader();
    if (!host.equals(WiFi.softAPIP().toString()) && host != "esp32-ota.local") {
      // Basic captive portal redirect
      _server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
      _server.send(302, "text/plain", "");
      return;
    }
  }

  database.format();
  _server.send(200, "text/html", _buildPage("Device successfully formatted, Restart the device now"));
}

void OTAProvisioner::_handleNotFound() {
  if (!_checkAuth()) return;
  // Redirect everything to root when using captive portal
  if (_useDNS) {
    _server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
    _server.send(302, "text/plain", "");
    return;
  }
  _server.send(404, "text/plain", "Not found");
}

void OTAProvisioner::_handleUpdate() {
  if (!_checkAuth()) return;

  if (Update.hasError()) {
    _server.send(200, "text/html", _buildPage("ERROR: update failed. Check serial logs."));
    return;
  }
  _server.send(200, "text/html", _buildPage("OK: update successful. Rebooting..."));

  // Give the browser a moment to show the message
  delay(800);
  ESP.restart();
}

void OTAProvisioner::_handleUpload() {
  if (!_checkAuth()) return;

  HTTPUpload& upload = _server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    // Begin OTA
    // If uploading app partition, size is unknown; Update will pick correct partition
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // Write received chunk
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (!Update.end(true)) {   // true = set this partition as bootable
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.abort();
  }
}
