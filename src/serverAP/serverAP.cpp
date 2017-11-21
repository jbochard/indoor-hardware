#include "serverAP.h"
#include "../utils/utils.h"
#include <FS.h>
#include <jsmn.h>

#define WIFI_FILE "wifi.conf"

const char WiFiAPPSK[] = "sparkfun";

ServerAP::ServerAP(std::shared_ptr<Display> d) {
  Serial.begin(9600);
  SPIFFS.begin();

  display = d;
  display->clear();
  display->printXY(4, 0, ">> Indoor <<");
}

void ServerAP::start() {
  ESP.eraseConfig();
  WiFi.mode(WIFI_AP);

  String ssid = buildSSID("davinci");
  String pwd = randomPWD(10);

  WiFi.softAP(ssid.c_str(), pwd.c_str());

  display->printXY(0, 1, "IP: 192.168.4.1     ");
  display->printXY(0, 3, "ssid: " + ssid);
  display->printXY(0, 4, "pwd: " + pwd);

  server.reset(new ESP8266WebServer(80));
  server->on("/config/wifi", HTTP_POST, [this]() {
    if (server->hasArg("plain")) {
      String plain = server->arg("plain");
      String body = configure(plain);
      if (body.indexOf("error") > 0) {
        server->send(404, "application/json", body);
      } else {
        server->send(200, "application/json", body);
      }
    } else {
      server->send(404, "application/json", "{ \"error\": \"Falta body\" }");
    }
  });
  server->begin();
}

void ServerAP::loop() {
  server->handleClient();
}

void ServerAP::stop() {
  server->close();
  WiFi.softAPdisconnect();
  WiFi.disconnect();
}

String ServerAP::buildSSID(String prefix, int max) {
  String ssid = prefix;
  byte numSsid = WiFi.scanNetworks();

  // print the network number and name for each network found:
  bool exists = false;
  do {
    exists = false;
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
      if (WiFi.SSID(thisNet) == ssid) {
        exists = true;
        ssid = prefix + "-" + random(0,50);
        break;
      }
    }
    max--;
  } while(exists && max > 0);
  return ssid;
}

String ServerAP::randomPWD(const int len) {
  String str = "";
  for (int i = 0; i < len; i++) {
    byte randomValue = random(0, 37);
    char letter = randomValue + 'a';
    if(randomValue > 26)
      letter = (randomValue - 26) + '0';
    str = str + letter;
  }
  return str;
}

String ServerAP::configure(String body) {
  jsmn_parser p;
  jsmntok_t t[6]; /* We expect no more than 128 tokens */
  char* buffer = strdup(body.c_str());

  jsmn_init(&p);
  int r = jsmn_parse(&p, buffer, strlen(buffer), t, sizeof(t)/sizeof(t[0]));
  if (r < 0) {
    free(buffer);
  	return "{ \"error\": \"El body no es un json\" }";
  }

	if (r < 1 || t[0].type != JSMN_OBJECT) {
    free(buffer);
    return "{ \"error\": \"El body no es un json\" }";
	}

  for (int i = 1; i < r; i++) {
		if (jsoneq(buffer, &t[i], "ssid") == 0) {
      ssid = charToString(buffer, t[i+1].start, t[i+1].end);
      continue;
		}
    if (jsoneq(buffer, &t[i], "password") == 0) {
      password = charToString(buffer, t[i+1].start, t[i+1].end);
      continue;
		}
    if (jsoneq(buffer, &t[i], "host") == 0) {
      host = charToString(buffer, t[i+1].start, t[i+1].end);
      continue;
		}
	}
  free(buffer);
  writeConfiguration();
  return "{ \"status\": \"OK\" }";
}

void ServerAP::writeConfiguration() {
  SPIFFS.remove(WIFI_FILE);
  File f = SPIFFS.open(WIFI_FILE, "w");
  if (f) {
    String buf = char(ssid.length()) + ssid + char(password.length()) + password + char(host.length()) + host;
    f.print(buf);
    f.close();
  }
}
