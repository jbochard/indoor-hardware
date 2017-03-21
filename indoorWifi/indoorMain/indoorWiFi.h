#ifndef H_INDOOR_WIFI
#define H_INDOOR_WIFI

#include <Arduino.h>

#define COMMAND_OK      0
#define COMMAND_UNKNOWN 1
#define COMMAND_ERROR   2

#define CONNECT_OK      0
#define CONNECT_ERROR   1

#define WIFI_FILE_NAME "/wifi_conf.json"

struct WiFiConfig {
  String  host;
  String  ssid;
  String  password;
  String error;
};

String readCommand();

void registerHost();

int executeCommand(String command);

int connectWiFi(WiFiConfig conf);

int wiFiStatus();

String readWiFi();

WiFiConfig parseWiFiConfig();

String jsonWiFiConfig(WiFiConfig wifiConfig);

bool deleteWiFiConfig();

void disconnectWiFi();

#endif

