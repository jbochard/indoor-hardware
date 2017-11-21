#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>

class Wifi {

private:
  String host;
  String ssid;
  String password;
  int status;
  int cnt;

  void readConfiguration();

public:
  Wifi();

  void begin();

  bool connect();

  void disconnect();

  String getJSON();

  String getHost();
};

#endif
