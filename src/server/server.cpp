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
  ESP.eraseConfig();
  displayState = 1;
  show();
  rules->begin();
  wifi->begin();
  wifi->connect();

  String host = wifi->getHost();
  if (MDNS.begin(host.c_str())) {
    MDNS.addService("http", "tcp", 80);
  }

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
    srv->send(200, "application/json", "{ \"clock\": \""+String(d.year)+"-"+d.month+"-"+d.day+"T"+d.hour+":"+d.minute+":"+d.second+"\" }");
  });

  srv->on("/hardware/clock", HTTP_POST, [this]() {
    ClockValue d = hardware->writeClock(srv->arg("year"),srv->arg("month"),srv->arg("day"),srv->arg("hour"),srv->arg("minute"),srv->arg("second"));
    String strClock = getDate(d) + "T" + getTime(d);
    srv->send(200, "application/json", "{ \"clock\": \"" + strClock + "\" }");
  });

  srv->on("/hardware/sensor", HTTP_GET, [this]() {
    if (srv->hasArg("name")) {
      String name = srv->arg("name");
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
        String name = hardware->getSensorName(i);
        if (i == 0) {
          buffer = "\"" + name + "\"";
        } else {
          buffer = buffer + ",\"" + name + "\"";
        }
      }
      srv->send(200, "application/json", "[" + buffer + "]");
    }
  });

  srv->on("/hardware/switch", HTTP_GET, [this]() {
    if (srv->hasArg("name")) {
      String name = srv->arg("name");
      HSWITCH sw = hardware->readSwitch(name);
      if (sw.name == "ERROR") {
        srv->send(400, "application/json", "{ \"error\": \"Switch "+name+" no existe.\" }");
      } else {
        srv->send(200, "application/json", "{ \"name\": \""+sw.name+"\", \"manual\": "+String(sw.manual, 4)+", \"value\": "+String(sw.state, 4)+" }");
      }
    } else {
      String buffer = String();
      for (int i = 0; i < hardware->getSwitchNumber(); i++) {
        String name = hardware->getSwitchName(i);
        if (i == 0) {
          buffer = "\"" + name + "\"";
        } else {
          buffer = buffer + ",\"" + name + "\"";
        }
      }
      srv->send(200, "application/json", "[" + buffer + "]");
    }
  });

  srv->on("/hardware/switch", HTTP_POST, [this]() {
    if (srv->hasArg("name") && srv->hasArg("plain")) {
      String name = srv->arg("name");
      String plain = srv->arg("plain");
      int res = hardware->switchUpdate(name, plain);
      if (res == 0) {
        srv->send(200, "application/json", "{ \"status\": \"OK\" }");
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

  /*
  srv->on("/hardware/config", HTTP_GET, [this]() {
    srv->send(200, "application/json", jsonHardware(true));
  });
  srv->on("/hardware/values", HTTP_GET, [this]() {
    srv->send(200, "application/json", jsonHardware(false));
  });
  srv->on("/hardware/write", HTTP_POST, [this]() {
    String name = srv->arg("name");
    String val = srv->arg("value");
    float value = setHardwareValue(name, val, true);
    if (value == SENSOR_DISABLED) {
      srv->send(400, "application/json", "{ \"error\": \"Sensor "+name+" esta deshabilidado.\" }");
    } else {
      srv->send(200, "application/json", "{ \"value\": "+String(value, 4)+" }");
    }
  });
  srv->on("/hardware/reset", HTTP_POST, [this]() {
    resetHardware();
    srv->send(200, "application/json", "{ \"value\": \"OK\" }");
  });
  srv->on("/hardware/enable", HTTP_POST, [this]() {
    String name = srv->arg("name");
    float value = enableHardware(name, true);
    if (value == SENSOR_DISABLED) {
      srv->send(400, "application/json", "{ \"error\": \"Sensor "+name+" esta deshabilidado.\" }");
    } else if (value == SENSOR_WRONG) {
      srv->send(400, "application/json", "{ \"error\": \"Sensor "+name+" no registrado.\" }");
    } else {
      srv->send(200, "application/json", "{ \"value\": \"OK\" }");
    }
  });
  srv->on("/hardware/disable", HTTP_POST, [this]() {
    String name = srv->arg("name");
    float value = enableHardware(name, false);
    if (value == SENSOR_DISABLED) {
      srv->send(400, "application/json", "{ \"error\": \"Sensor "+name+" esta deshabilidado.\" }");
    } else if (value == SENSOR_WRONG) {
      srv->send(400, "application/json", "{ \"error\": \"Sensor "+name+" no registrado.\" }");
    } else {
      srv->send(200, "application/json", "{ \"value\": \"OK\" }");
    }
  });
  srv->on("/hardware/manual", HTTP_POST, [this]() {
    String name = srv->arg("name");
    float value = manualHardware(name, true);
    if (value == SENSOR_DISABLED) {
      srv->send(400, "application/json", "{ \"error\": \"Sensor "+name+" esta deshabilidado.\" }");
    } else if (value == SENSOR_WRONG) {
      srv->send(400, "application/json", "{ \"error\": \"Sensor "+name+" no registrado.\" }");
    } else {
      srv->send(200, "application/json", "{ \"value\": \"OK\" }");
    }
  });
  srv->on("/hardware/auto", HTTP_POST, [this]() {
    String name = srv->arg("name");
    float value = manualHardware(name, false);
    if (value == SENSOR_DISABLED) {
      srv->send(400, "application/json", "{ \"error\": \"Sensor "+name+" esta deshabilidado.\" }");
    } else if (value == SENSOR_WRONG) {
      srv->send(400, "application/json", "{ \"error\": \"Sensor "+name+" no registrado.\" }");
    } else {
      srv->send(200, "application/json", "{ \"value\": \"OK\" }");
    }
  });
*/
/*
  srv->on("/rules", HTTP_GET, [this]() {
    String json = listJsonRules();
    srv->send(200, "application/json", json);
  });

  srv->on("/rules", HTTP_POST, [this]() {
    String rule = srv->arg("plain");
    insertRule(rule);
    srv->send(200, "application/json", "{ \"value\": \"OK\" }");
  });

  srv->on("/rules", HTTP_DELETE, [this]() {
    if (srv->hasArg("idx")) {
      String idx = srv->arg("idx");
      deleteRule(idx.toInt());
    } else {
      deleteRules();
    }
    srv->send(200, "application/json", "{ \"value\": \"OK\" }");
  });

  srv->on("/rules/reset", HTTP_POST, [this]() {
    deleteRules();
    srv->send(200, "application/json", "{ \"value\": \"OK\" }");
  });
  */
  srv->begin();
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
    display->printXY(0, 3, "Fecha: " + getDate(clock));
    display->printXY(0, 4, " Hora:  " + getTime(clock));
    return;
  }
  if (displayState == 3) {
    for (int i = 0; i < hardware->getSensorNumber(); i++) {
      String name = hardware->getSensorName(i);
      display->printXY(0, i+1, hardware->printSensor(name));
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
