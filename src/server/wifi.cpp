#include "wifi.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <jsmn.h>
#include "../utils/utils.h"

#define WIFI_FILE "wifi.conf"
#define WIFI_DELAY  30

Wifi::Wifi() {
  cnt = -1;
  Serial.begin(9600);
  SPIFFS.begin();
  readConfiguration();
}

void Wifi::begin() {
  char b_ssid[ssid.length()+1];
  char b_pass[password.length()+1];
  ssid.toCharArray(b_ssid, ssid.length()+1);
  password.toCharArray(b_pass, password.length()+1);

//  WiFi.persistent(false);
//  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);    //if it gets disconnected
  WiFi.begin(b_ssid, b_pass);
  delay(1000);
}

bool Wifi::connect() {
  cnt = (cnt + 1) % WIFI_DELAY;
  if (cnt == 0) {
    status = WiFi.status();
    host = WiFi.localIP().toString();
  }
  return status == WL_CONNECTED;
}

void Wifi::disconnect() {
  WiFi.disconnect();
  status = WiFi.status();
  cnt = -1;
}

String Wifi::getJSON() {
  return "{\"ssid\":\""+ssid+"\",\"password\":\"***********\",\"host\":\""+host+"\"}";
}

String Wifi::getHost() {
  return host;
}

void Wifi::readConfiguration() {
  File file = SPIFFS.open(WIFI_FILE, "r");
  if (!file) {
    return;
  }
  size_t size = file.size();
  std::unique_ptr<char[]> buf(new char[size]);

  file.readBytes(buf.get(), size);

  int ssid_size = (int)buf[0];
  int pwd_size = (int)buf[ssid_size + 1];
  int host_size = (int)buf[ssid_size + pwd_size + 2];
  ssid = charToString(buf.get(), 1, ssid_size+1);
  password = charToString(buf.get(), ssid_size+2, ssid_size + pwd_size + 2);
  host = charToString(buf.get(), ssid_size + pwd_size + 3, size);

  file.close();
}
