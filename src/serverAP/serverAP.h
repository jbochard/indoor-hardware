#ifndef SERVER_AP_H
#define SERVER_AP_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "../components/display.h"

class ServerAP {

private:
  String ssid;
  String password;
  String host;
  std::unique_ptr<ESP8266WebServer> server;
  std::shared_ptr<Display>          display;

  String configure(String body);

  String randomPWD(const int len);

  String buildSSID(String prefix, int max=10);

  void writeConfiguration();

public:
  ServerAP(std::shared_ptr<Display> d);

  void start();

  void loop();

  void stop();

};

#endif
