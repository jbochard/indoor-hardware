#include "server.h"
#include <math.h>
#include "../utils/utils.h"

ServerIndoor::ServerIndoor() {
  hardware = std::make_shared<Hardware>();
  rules.reset(new Rules(hardware));
  srv.reset(new ESP8266WebServer(80));

  hardware->begin();
  rules->begin();
  configureServer();
}

bool ServerIndoor::start() {

  DEBUG_WM(F(""));
  DEBUG_WM(F("Start..."));
  if (_callback) {
    _callback(this, {STA, DISCONNECTED, "", "", ""});
  }

  WiFi.mode(WIFI_STA);

  if (connectWifi("", "") == WL_CONNECTED)   {
    String ip = WiFi.localIP().toString();
    String ssid = WiFi.SSID();
    DEBUG_WM(F("IP Address:"));
    DEBUG_WM(WiFi.localIP());
    if (_callback) {
      _callback(this, { STA, CONNECTED, ssid, "", ip });
    }
    //connected
    return true;
  }

  String ssid = buildSSID("davinci", 10);
  String pwd = randomPWD(10);
  return startConfigPortal(ssid.c_str(), pwd.c_str());
}

void ServerIndoor::loop() {
  hardware->update();
  rules->update();
  srv->handleClient();
}

void ServerIndoor::stop() {
  srv->close();
  hardware.reset();
  srv.reset();
}

void ServerIndoor::setConnectTimeout(unsigned long connectTimeout) {
  _connectTimeout = connectTimeout;
}

void ServerIndoor::setConfigPortalTimeout(unsigned long connectTimeout) {
  _configPortalTimeout = connectTimeout;
}

void ServerIndoor::setCallback(void (*func)(ServerIndoor*, ServerState)) {
  _callback = func;
}

String ServerIndoor::getSSID() {
  return WiFi.SSID();
}

String ServerIndoor::getIP() {
  return WiFi.localIP().toString();
}

Hardware* ServerIndoor::hw() {
  return hardware.get();
}

void ServerIndoor::configureServer() {

    srv->on("/config/wifi", HTTP_GET, [this]() {
      srv->send(200, "application/json",  "{\"ssid\":\""+WiFi.SSID()+"\",\"password\":\"***********\",\"host\":\""+WiFi.localIP().toString()+"\"}");
    });

    srv->on("/hardware/clock", HTTP_GET, [this]() {
      ClockValue d = hardware->readClock();
      String strClock = getDateYMD(d) + "T" + getTime(d) + "Z";
      srv->send(200, "application/json", "{ \"clock\": \"" + strClock + "\" }");
    });

    srv->on("/hardware/clock", HTTP_POST, [this]() {
      if (srv->hasArg("plain")) {
        String plain = srv->arg("plain");
        ClockValue d = hardware->writeClock(plain);
        String strClock = getDateYMD(d) + "T" + getTime(d) + "Z";
        srv->send(200, "application/json", "{ \"clock\": \"" + strClock + "\" }");
      } else {
        srv->send(400, "application/json", "{ \"error\": \"Falta body\" }");
      }
    });

    srv->on("/hardware/sensor", HTTP_GET, [this]() {
      if (srv->hasArg("type")) {
        String name = srv->arg("type");
        HSENSOR sensor = hardware->readSensor(name);
        if (sensor.name == "ERROR") {
          srv->send(400, "application/json", "{ \"error\": \"Sensor "+name+" no existe.\" }");
        } else if (isnan(sensor.value)) {
            srv->send(400, "application/json", "{ \"error\": \"Sensor "+name+" retorno NaN.\" }");
        } else {
          srv->send(200, "application/json", "{ \"name\": \""+sensor.name+"\", \"value\": "+String(sensor.value, 4)+" }");
        }
      } else {
        String buffer = String();
        for (int i = 0; i < hardware->getSensorNumber(); i++) {
          HSENSOR sensor = hardware->readSensor(i);
          String value = (isnan(sensor.value))?"-1":String(sensor.value);
          if (i == 0) {
            buffer = "{ \"type\": \"" + sensor.name + "\", \"value\": " + value + " }";
          } else {
            buffer = buffer + ", { \"type\": \"" + sensor.name + "\", \"value\": " + value + " }";
          }
        }
        srv->send(200, "application/json", "[" + buffer + "]");
      }
    });

    srv->on("/hardware/switch", HTTP_GET, [this]() {
      if (srv->hasArg("type")) {
        String name = srv->arg("type");
        HSWITCH sw = hardware->readSwitch(name);
        if (sw.name == "ERROR") {
          srv->send(400, "application/json", "{ \"error\": \"Switch "+name+" no existe.\" }");
        } else {
          srv->send(200, "application/json", "{ \"type\": \""+sw.name+"\", \"manual\": "+String(sw.manual, 4)+", \"value\": "+String(sw.state, 4)+" }");
        }
      } else {
        String buffer = String();
        for (int i = 0; i < hardware->getSwitchNumber(); i++) {
          HSWITCH rsw = hardware->readSwitch(i);
          if (i == 0) {
            buffer = "{ \"type\": \"" + rsw.name + "\", \"manual\": "+String(rsw.manual)+", \"value\": "+String(rsw.state)+" }";
          } else {
            buffer = buffer + ", { \"type\": \"" + rsw.name + "\", \"manual\": "+String(rsw.manual)+", \"value\": "+String(rsw.state)+" }";
          }
        }
        srv->send(200, "application/json", "[" + buffer + "]");
      }
    });

    srv->on("/hardware/switch", HTTP_POST, [this]() {
      if (srv->hasArg("type") && srv->hasArg("plain")) {
        String name = srv->arg("type");
        String plain = srv->arg("plain");
        HSWITCH rsw = hardware->switchUpdate(name, plain);
        if (rsw.name != "ERROR") {
          srv->send(200, "application/json", "{ \"type\": \"" + rsw.name + "\", \"manual\": "+String(rsw.manual)+", \"value\": "+String(rsw.state)+" }");
        } else {
          srv->send(400, "application/json", "{ \"error\": \"Error la parsear entrada.\" }");
        }
      } else {
        srv->send(400, "application/json", "{ \"error\": \"Debe agregar un parámetro name con el nombre del switch a cambiar.\" }");
      }
    });

    srv->on("/rules", HTTP_GET, [this]() {
      String json = rules->toJSON();
      srv->send(200, "application/json", json);
    });

    srv->on("/rules", HTTP_POST, [this]() {
      String rule = srv->arg("plain");
      rules->configure(rule);
      srv->send(200, "application/json", "{ \"value\": \"OK\" }");
    });

    srv->on("/test", HTTP_GET, [this]() {
      srv->send(200, "application/json", "{ \"mode\": \"NORMAL\" }");
    });

    srv->on("/hardware/switch", HTTP_OPTIONS, [this]() {
      srv->sendHeader("access-control-allow-credentials", "false");
      srv->sendHeader("access-control-allow-headers", "Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers");
      srv->sendHeader("access-control-allow-methods", "GET,POST,OPTIONS");
      srv->send(204, "application/json");
    });

    srv->on("/hardware/clock", HTTP_OPTIONS, [this]() {
      srv->sendHeader("access-control-allow-credentials", "false");
      srv->sendHeader("access-control-allow-headers", "Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers");
      srv->sendHeader("access-control-allow-methods", "GET,POST,OPTIONS");
      srv->send(204, "application/json");
    });

    srv->on("/rules", HTTP_OPTIONS, [this]() {
      srv->sendHeader("access-control-allow-credentials", "false");
      srv->sendHeader("access-control-allow-headers", "Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers");
      srv->sendHeader("access-control-allow-methods", "GET,POST,OPTIONS");
      srv->send(204, "application/json");
    });

    srv->begin();
}

bool ServerIndoor::startConfigPortal(char const *apName, char const *apPassword) {
  //setup AP
  WiFi.mode(WIFI_AP_STA);
  DEBUG_WM("SET AP STA");

  _apName = apName;
  _apPassword = apPassword;

  connect = false;
  setupConfigPortal();

  while(1) {

    // check if timeout
    if(configPortalHasTimeout()) break;

    //DNS
    dnsServer->processNextRequest();
    //HTTP
    srv->handleClient();

    if (connect) {
      connect = false;
      delay(2000);
      DEBUG_WM(F("Connecting to new AP"));

      // using user-provided  _ssid, _pass in place of system-stored ssid and pass
      if (connectWifi(_ssid, _pass) != WL_CONNECTED) {
        DEBUG_WM(F("Failed to connect."));
      } else {
        //connected
        WiFi.mode(WIFI_STA);
        break;
      }
    }
    yield();
  }

  srv.reset();
  dnsServer.reset();
  return  WiFi.status() == WL_CONNECTED;
}

bool ServerIndoor::configPortalHasTimeout() {
    if(_configPortalTimeout == 0 || wifi_softap_get_station_num() > 0){
      _configPortalStart = millis(); // kludge, bump configportal start time to skew timeouts
      return false;
    }
    return (millis() > _configPortalStart + _configPortalTimeout);
}

void ServerIndoor::setupConfigPortal() {
  dnsServer.reset(new DNSServer());
  srv.reset(new ESP8266WebServer(80));

  DEBUG_WM(F(""));
  _configPortalStart = millis();

  DEBUG_WM(F("Configuring access point... "));
  DEBUG_WM(_apName);
  if (_apPassword != NULL) {
    if (strlen(_apPassword) < 8 || strlen(_apPassword) > 63) {
      // fail passphrase to short or long!
      DEBUG_WM(F("Invalid AccessPoint password. Ignoring"));
      _apPassword = NULL;
    }
    DEBUG_WM(_apPassword);
  }

  //optional soft ip config
  /*if (_ap_static_ip) {
    DEBUG_WM(F("Custom AP IP/GW/Subnet"));
    WiFi.softAPConfig(_ap_static_ip, _ap_static_gw, _ap_static_sn);
  }*/

  if (_apPassword != NULL) {
    WiFi.softAP(_apName, _apPassword);//password option
  } else {
    WiFi.softAP(_apName);
  }

  delay(500); // Without delay I've seen the IP address blank
  DEBUG_WM(F("AP IP address: "));
  DEBUG_WM(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

  srv->on("/config/wifi", HTTP_POST, [this]() {
    if (srv->hasArg("plain")) {
      String plain = srv->arg("plain");
      String body = updateWifi(plain);
      if (body.indexOf("error") > 0) {
        srv->send(404, "application/json", body);
      } else {
        srv->send(200, "application/json", body);
      }
    } else {
      srv->send(404, "application/json", "{ \"error\": \"Falta body\" }");
    }
  });

  srv->on("/test", HTTP_GET, [this]() {
    srv->send(200, "application/json", "{ \"mode\": \"CONFIG\" }");
  });

  srv->on("/config/wifi", HTTP_OPTIONS, [this]() {
    srv->sendHeader("access-control-allow-credentials", "false");
    srv->sendHeader("access-control-allow-headers", "Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers");
    srv->sendHeader("access-control-allow-methods", "POST,OPTIONS");
    srv->send(204, "application/json");
  });

  srv->begin();
  DEBUG_WM(F("HTTP server started"));
}

int ServerIndoor::connectWifi(String ssid, String pass) {
  DEBUG_WM(F("Connecting as wifi client..."));
  // check if we've got static_ip settings, if we do, use those.
  /*if (_sta_static_ip) {
    DEBUG_WM(F("Custom STA IP/GW/Subnet"));
    WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
    DEBUG_WM(WiFi.localIP());
  }*/
  //fix for auto connect racing issue
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_WM("Already connected. Bailing out.");
    return WL_CONNECTED;
  }
  //check if we have ssid and pass and force those, if not, try with last saved values
  if (ssid != "") {
    WiFi.begin(ssid.c_str(), pass.c_str());
    if (_callback) {
      _callback(this, {STA, WAITING_CONNECT, ssid, "", ""});
    }
  } else {
    if (WiFi.SSID()) {
      DEBUG_WM("Using last saved values, should be faster");
      DEBUG_WM("SSID: [" + WiFi.SSID() + "]");
      DEBUG_WM("pwd: [" + WiFi.psk() + "]");
      //trying to fix connection in progress hanging
      ETS_UART_INTR_DISABLE();
      wifi_station_disconnect();
      ETS_UART_INTR_ENABLE();

      WiFi.begin();
      if (_callback) {
        _callback(this, {STA, WAITING_CONNECT, ssid, "", ""});
      }
    } else {
      DEBUG_WM("No saved credentials");
      if (_callback) {
        _callback(this, {STA, DISCONNECTED, "", "", ""});
      }
    }
  }

  int connRes = waitForConnectResult();
  DEBUG_WM ("Connection result: ");
  DEBUG_WM ( connRes );
  return connRes;
}

uint8_t ServerIndoor::waitForConnectResult() {
  if (_connectTimeout == 0) {
    return WiFi.waitForConnectResult();
  } else {
    DEBUG_WM (F("Waiting for connection result with time out"));
    unsigned long start = millis();
    boolean keepConnecting = true;
    uint8_t status;
    while (keepConnecting) {
      status = WiFi.status();
      if (millis() > start + _connectTimeout) {
        keepConnecting = false;
        DEBUG_WM (F("Connection timed out"));
      }
      if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
        keepConnecting = false;
      }
      delay(100);
    }
    return status;
  }
}

String ServerIndoor::updateWifi(String body) {
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
      _ssid = charToString(buffer, t[i+1].start, t[i+1].end);
      DEBUG_WM ("Set SSID: [" + _ssid + "]");
      continue;
		}
    if (jsoneq(buffer, &t[i], "password") == 0) {
      _pass = charToString(buffer, t[i+1].start, t[i+1].end);
      DEBUG_WM ("Set pwd: [" + _pass + "]");
      continue;
		}
	}
  free(buffer);
  connect = true;
  return "{ \"status\": \"OK\" }";
}


String ServerIndoor::buildSSID(String prefix, int max) {
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

String ServerIndoor::randomPWD(const int len) {
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

template <typename Generic>
void ServerIndoor::DEBUG_WM(Generic text) {
  if (_debug) {
    Serial.print("*SI: ");
    Serial.println(text);
  }
}
