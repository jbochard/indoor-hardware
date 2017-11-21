#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include "../components/display.h"
#include "wifi.h"
#include "hardware.h"
#include "rules.h"

class ServerIndoor {

private:
  int cnt;
  int displayState;
  std::unique_ptr<Wifi>             wifi;
  std::unique_ptr<ESP8266WebServer> srv;
  std::unique_ptr<Rules>            rules;
  std::shared_ptr<Hardware>         hardware;
  std::shared_ptr<Display>          display;

  void show();

public:
  ServerIndoor(std::shared_ptr<Display> d);

  void start();

  void click();

  bool loop();

  void stop();
};

#endif
