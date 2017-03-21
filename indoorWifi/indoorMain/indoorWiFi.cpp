#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "indoorWiFi.h"
#include "jsonFS.h"

struct Iter {
  String first;
  String rest;
};

String buffer = "";

Iter scan(String text, char ch);
int configWiFiCommand(String command);
int resetWiFiCommand();

String readCommand() {
  char ch = 0;
  if (! Serial.available()) {
    return "";
  }
  ch = Serial.read();
  if (ch != '\n') {
    buffer.concat(ch);
    return "";
  } else {
    String result = String(buffer);
    buffer = "";
    result.trim();
    return result;    
  }  
}

int executeCommand(String command) {
  Iter token = scan(command, ' ');
  if (token.first == "config") {
    token = scan(token.rest, ' ');
    if (token.first == "server") {
      return configWiFiCommand(token.rest);
    }
  }
  if (token.first == "reset") {
    token = scan(token.rest, ' ');
    if (token.first == "server") {
      return resetWiFiCommand();
    }    
  }
  return COMMAND_UNKNOWN;     
}

void disconnectWiFi() {
  WiFi.disconnect();  
}

int connectWiFi(WiFiConfig conf) {
  Serial.println("Init...");
  char ssid[conf.ssid.length()+1];
  char pass[conf.password.length()+1];
  conf.ssid.toCharArray(ssid, conf.ssid.length()+1);
  conf.password.toCharArray(pass, conf.password.length()+1);
   
  int wifiStatus = WiFi.begin(ssid, pass);
  delay(5000);
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connection OK");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    return CONNECT_OK;
  } else {
    Serial.print("Error connecting to: ");
    Serial.println(WiFi.SSID());
    Serial.println(wifiStatus);
    WiFi.disconnect();
    return CONNECT_ERROR;
  }          
}

int wiFiStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    return CONNECT_OK;
  } else {
    return CONNECT_ERROR;    
  }
}

String readWiFi() {
  return readFile(WIFI_FILE_NAME);  
}

void registerHost() {
  char host[16];
  
  WiFiConfig conf = parseWiFiConfig();
  
  conf.host.toCharArray(host, 15);

  if (MDNS.begin(host)) {
    MDNS.addService("http", "tcp", 80);
  }  
}
WiFiConfig parseWiFiConfig() {
  WiFiConfig conf = { "", "", "", "" };
    String body = readFile(WIFI_FILE_NAME);
   JsonObject & json = jsonObject(body);
  if (json.containsKey("error")) {
    conf.error = json.get<String>("error");
  }
  conf.host = json.get<String>("host");
  conf.ssid = json.get<String>("ssid");
  conf.password = json.get<String>("password");
  return conf;
}

String jsonWiFiConfig(WiFiConfig wiFiConfig) {
  return "{\"ssid\":\""+wiFiConfig.ssid+"\",\"password\":\""+wiFiConfig.password+"\",\"host\":\""+wiFiConfig.host+"\"}";  
}

bool deleteWiFiConfig() {
  return SPIFFS.remove(WIFI_FILE_NAME);
}

// ------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------


int configWiFiCommand(String command) {
  WiFiConfig conf;
  Iter token = scan(command, ' ');
  conf.host = token.first;
  
  token = scan(token.rest, ' ');
  conf.ssid = token.first;
  
  token = scan(token.rest, ' ');
  conf.password = token.first;

  String body = jsonWiFiConfig(conf);
  writeFile(WIFI_FILE_NAME, body);
  return COMMAND_OK;
}

int resetWiFiCommand() {
  if (deleteWiFiConfig()) {
    return COMMAND_OK;
  }
}

Iter scan(String text, char ch) {
  Iter res;
  int idx = text.indexOf(ch);
  if (idx < 0) {
    res.first = text;
    res.rest = "";
    return res;
  }
  res.first = text.substring(0, idx);
  res.rest = text.substring(idx+1);
  return res;
}

