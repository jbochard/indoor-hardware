#include "server.h"
#include "hardware.h"
#include "jsonFS.h"
#include "indoorWiFi.h"
#include "utils.h"
#include "rules.h"
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

// ---------------------------------- ServerWiFi --------------------------------------


int serverConfig() {
      
  server.on("/config/wifi", HTTP_GET, []() {
      String body = readWiFi();
      if (body.indexOf("error") > 0) {
        server.send(404, "application/json", body);      
      } else {
        server.send(200, "application/json", body);
      }
  });
  
  server.on("/hardware/clock", HTTP_GET, []() {
    String json = loadClock();
    server.send(200, "application/json", json);      
  }); 
  server.on("/hardware/clock", HTTP_POST, []() {
     if (setClock(server.arg("year"),server.arg("month"),server.arg("day"),server.arg("hour"),server.arg("minute"),server.arg("second"))) {
      server.send(200, "application/json", "{ \"value\": \"OK\" }");
    } else {
      server.send(400, "application/json", "{ \"error\": \"Fecha/Hora no registrado.\" }");      
    }
  }); 
  server.on("/hardware/read", HTTP_GET, []() {
    String name = server.arg("name");
    float value = getHardwareValue(name);
    if (value == SENSOR_DISABLED) {
      server.send(400, "application/json", "{ \"error\": \"Sensor "+name+" esta deshabilidado.\" }");      
    } else if (value == SENSOR_WRONG) {
      server.send(400, "application/json", "{ \"error\": \"Sensor "+name+" no registrado.\" }");      
    } else {
      server.send(200, "application/json", "{ \"value\": "+String(value, 4)+" }");      
    }
  });
  server.on("/hardware/config", HTTP_GET, []() {
    server.send(200, "application/json", jsonHardware(true));      
  });
  server.on("/hardware/values", HTTP_GET, []() {
    server.send(200, "application/json", jsonHardware(false));      
  });  
  server.on("/hardware/write", HTTP_POST, []() {
    String name = server.arg("name");
    String val = server.arg("value");
    float value = setHardwareValue(name, val, true);
    if (value == SENSOR_DISABLED) {
      server.send(400, "application/json", "{ \"error\": \"Sensor "+name+" esta deshabilidado.\" }");      
    } else {
      server.send(200, "application/json", "{ \"value\": "+String(value, 4)+" }");      
    }
  });   
  server.on("/hardware/reset", HTTP_POST, []() {
    resetHardware();
    server.send(200, "application/json", "{ \"value\": \"OK\" }");
  });        
  server.on("/hardware/enable", HTTP_POST, []() {
    String name = server.arg("name");
    float value = enableHardware(name, true);
    if (value == SENSOR_DISABLED) {
      server.send(400, "application/json", "{ \"error\": \"Sensor "+name+" esta deshabilidado.\" }");      
    } else if (value == SENSOR_WRONG) {
      server.send(400, "application/json", "{ \"error\": \"Sensor "+name+" no registrado.\" }");      
    } else {
      server.send(200, "application/json", "{ \"value\": \"OK\" }");      
    }
  });    
  server.on("/hardware/disable", HTTP_POST, []() {
    String name = server.arg("name");
    float value = enableHardware(name, false);
    if (value == SENSOR_DISABLED) {
      server.send(400, "application/json", "{ \"error\": \"Sensor "+name+" esta deshabilidado.\" }");      
    } else if (value == SENSOR_WRONG) {
      server.send(400, "application/json", "{ \"error\": \"Sensor "+name+" no registrado.\" }");      
    } else {
      server.send(200, "application/json", "{ \"value\": \"OK\" }");      
    }
  }); 
  server.on("/hardware/manual", HTTP_POST, []() {
    String name = server.arg("name");
    float value = manualHardware(name, true);
    if (value == SENSOR_DISABLED) {
      server.send(400, "application/json", "{ \"error\": \"Sensor "+name+" esta deshabilidado.\" }");      
    } else if (value == SENSOR_WRONG) {
      server.send(400, "application/json", "{ \"error\": \"Sensor "+name+" no registrado.\" }");      
    } else {
      server.send(200, "application/json", "{ \"value\": \"OK\" }");      
    }
  });   
  server.on("/hardware/auto", HTTP_POST, []() {
    String name = server.arg("name");
    float value = manualHardware(name, false);
    if (value == SENSOR_DISABLED) {
      server.send(400, "application/json", "{ \"error\": \"Sensor "+name+" esta deshabilidado.\" }");      
    } else if (value == SENSOR_WRONG) {
      server.send(400, "application/json", "{ \"error\": \"Sensor "+name+" no registrado.\" }");      
    } else {
      server.send(200, "application/json", "{ \"value\": \"OK\" }");      
    }
  }); 
  server.on("/rules", HTTP_GET, []() {
    String json = listJsonRules();
    server.send(200, "application/json", json);      
  });  
  server.on("/rules", HTTP_POST, []() {
    String rule = server.arg("rule");
    insertRule(rule);
    server.send(200, "application/json", "{ \"value\": \"OK\" }");
  });   
  server.on("/rules", HTTP_DELETE, []() {
    if (server.hasArg("idx")) {
      String idx = server.arg("idx");
      deleteRule(idx.toInt());      
    } else {
      deleteRules();      
    }
    server.send(200, "application/json", "{ \"value\": \"OK\" }");
  });  
  server.begin();
  return SERVER_STARTED;  
}

void serverLoop() {
  server.handleClient();  
}



