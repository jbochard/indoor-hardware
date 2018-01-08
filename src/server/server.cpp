#include "server.h"
#include <math.h>
#include "../utils/utils.h"

ServerIndoor::ServerIndoor(std::shared_ptr<Display> d) {
  display = d;
  displayState = 0;
  hardware = std::make_shared<Hardware>();
  rules.reset(new Rules(hardware));
  wifi.reset(new Wifi());
  srv.reset(new ESP8266WebServer(80));

  hardware->begin();
}

void ServerIndoor::start() {
  display->clear();
  displayState = 1;
  show();
  rules->begin();
  wifi->begin();
  wifi->connect();

  String host = wifi->getHost();
  MDNS.begin(host.c_str());

  srv->on("/config/wifi", HTTP_GET, [this]() {
    String body = wifi->getJSON();
    if (body.indexOf("error") > 0) {
      srv->send(404, "application/json", body);
    } else {
      srv->send(200, "application/json", body);
    }
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

  srv->on("/display/dot", HTTP_POST, [this]() {
    display->dotXY(srv->arg("x").toInt(), srv->arg("y").toInt());
    srv->send(200, "application/json", "{ \"status\": \"OK\" }");
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
  MDNS.addService("http", "tcp", 80);
}

void ServerIndoor::click() {
  display->clear();
  if (displayState >= 2) {
    displayState++;
    if (displayState == 5) {
      displayState = 2;
    }
  }
}

bool ServerIndoor::loop() {
  bool res = wifi->connect();
  if (displayState == 1 && res) {
    displayState = 2;
  }
  if (displayState > 1 && !res) {
    displayState = 1;
  }
  show();
  hardware->update();
  rules->update();
  srv->handleClient();
  return displayState > 0;
}

void ServerIndoor::show() {
  display->printXY(4, 0, ">> Indoor <<");
  if (displayState == 0) {
    display->printXY(0, 1, "Desconectado.       ");
    return;
  }
  if (displayState == 1) {
    display->printXY(0, 1, "Conectando...        ");
    return;
  }
  if (displayState == 2) {
    String host = wifi->getHost();
    ClockValue clock = hardware->readClock();
    display->printXY(0, 1, "IP: " + host);
    display->printXY(0, 2, "                    ");
    display->printXY(0, 3, "Fecha: " + getDateDMY(clock));
    display->printXY(0, 4, " Hora:  " + getTime(clock));
    return;
  }
  if (displayState == 3) {
    for (int i = 0; i < hardware->getSensorNumber(); i++) {
      String name = hardware->getSensorName(i);
      String buf = hardware->printSensor(name);
      display->printXY(0, i+1, buf);
    }
    return;
  }
  if (displayState == 4) {
    for (int i = 0; i < hardware->getSwitchNumber(); i++) {
      String name = hardware->getSwitchName(i);
      display->printXY(0, i+1, hardware->printSwitch(name));
    }
    return;
  }
}

void ServerIndoor::stop() {
  displayState = 0;
  srv->close();
  wifi->disconnect();
  hardware.reset();
  wifi.reset();
  srv.reset();
}
