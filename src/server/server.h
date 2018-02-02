#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <memory>
#include "wifi.h"
#include "hardware.h"
#include "rules.h"

extern "C" {
  #include "user_interface.h"
}
#define DISCONNECTED     0
#define WAITING_CONNECT  1
#define CONNECTED        2
#define STA              "STA"
#define APM              "APM"

struct ServerState {
  String mode;
  int state;
  String ssid;
  String pwd;
  String ip;
};

class ServerIndoor {

private:
  std::unique_ptr<DNSServer>        dnsServer;
  std::unique_ptr<ESP8266WebServer> srv;
  std::unique_ptr<Rules>            rules;
  std::shared_ptr<Hardware>         hardware;
  bool                              _debug = true;

  const char*   _apName                 = "no-net";
  const char*   _apPassword             = NULL;
  String        _ssid                   = "";
  String        _pass                   = "";
  unsigned long _configPortalTimeout    = 0;
  unsigned long _connectTimeout         = 0;
  unsigned long _configPortalStart      = 0;
  bool           connect;

  void (*_callback)(ServerIndoor*, ServerState) = NULL;

  // DNS server
  const byte    DNS_PORT = 53;

  int connectWifi(String ssid, String pass);

  String updateWifi(String body);

  void configureServer();

  bool startConfigPortal(char const *apName, char const *apPassword);

  void setupConfigPortal();

  bool configPortalHasTimeout();

  uint8_t waitForConnectResult();

  String buildSSID(String prefix, int max);

  String randomPWD(const int len);

public:
  ServerIndoor();

  bool start();

  void loop();

  void stop();

  void setConnectTimeout(unsigned long connectTimeout);

  void setConfigPortalTimeout(unsigned long configPortalTimeout);

  void setCallback(void (*func)(ServerIndoor*, ServerState));

  String getSSID();

  String getIP();

  Hardware* hw();

  template <typename Generic>
  void          DEBUG_WM(Generic text);

};

#endif
