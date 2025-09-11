#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <DNSServer.h>
#include <ESPmDNS.h>

class OTAProvisioner {
public:
  // Call this in setup() to start AP + web UI for OTA
  // Pass NULL or "" for password to create an open AP.
  // If captivePortal = true, a simple DNS server will redirect most requests to 192.168.4.1
  bool beginAP(const char* ssid = "ESP32-Provision",
               const char* password = "",
               bool captivePortal = true,
               IPAddress apIP = IPAddress(192,168,4,1),
               IPAddress gateway = IPAddress(192,168,4,1),
               IPAddress subnet = IPAddress(255,255,255,0));

  // Call in loop()
  void loop();

  // Optional: set a basic-auth credential for the upload page
  void setBasicAuth(const char* user, const char* pass);

private:
  WebServer _server{80};
  DNSServer _dns;
  bool _useDNS = false;
  bool _useAuth = false;
  String _authUser;
  String _authPass;

  void _registerRoutes();
  void _handleRoot();
  void _handleNotFound();
  void _handleUpdate();
  void _handleUpload();

  bool _checkAuth();
  String _buildPage(const String& msg = String());
};

extern OTAProvisioner ota;