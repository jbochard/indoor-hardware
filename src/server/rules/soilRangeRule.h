#ifndef SOIL_RANGE_RULE_H
#define SOIL_RANGE_RULE_H

#include <Arduino.h>
#include <memory>
#include <jsmn.h>
#include "rule.h"
#include "../../utils/utils.h"

class SoilRangeRule : public Rule {

public:
  SoilRangeRule(std::shared_ptr<Hardware> h) : Rule(h) {
    relayName = "Relay1";
    enabled = false;
    soilIni = 0;
    soilEnd = 0;
  }

  String name() {
    return "soilRange";
  }

  void execute() {
    if (enabled) {
      bool apply = false;
      HSENSOR sensor = hardware->readSensor("SoilMoisture");
      if (soilIni == 0) {
        apply = sensor.value <= soilEnd;
      }
      if (soilEnd == 0) {
        apply = sensor.value >= soilIni;
      }
      if (soilIni > 0 && soilEnd > 0) {
        apply = sensor.value >= soilIni && sensor.value <= soilEnd;
      }
      hardware->writeSwitch(relayName, "value", apply, true);
    }
  }

  // S0010025Relay1
  bool configureBuffer(String buf) {
    if (buf.charAt(0) == 'S') {
      enabled    = buf.substring(1, 2).toInt();
      soilIni    = buf.substring(2, 5).toInt();
      soilEnd    = buf.substring(5, 8).toInt();
      relayName  = buf.substring(8, 14);
      return true;
    }
    return false;
  }

  String toRegister() {
    char buf[14];
    sprintf(buf, "S%d%3s%3s%5s", enabled, soilIni, soilEnd, relayName.c_str());
    return charToString(buf, 0, strlen(buf));
  }

  String toJSON() {
    return "{ \"type\": \"soilRange\",  \"relayName\": \""+relayName+"\", \"enabled\": "+((enabled)?"true":"false")+", \"soilIni\": "+String(soilIni)+", \"soilEnd\": "+String(soilEnd)+" }";
  }

  String getProperty(String key) {
    if (key == "enabled") {
      return ((enabled)?"true":"false");
    }
    if (key == "relayName") {
      return relayName;
    }
    if (key == "soilIni") {
      return String(soilIni);
    }
    if (key == "soilEnd") {
      return String(soilEnd);
    }
    return "";
  }
  void setProperty(String key, String value) {
    if (key == "relayName") {
      relayName = value;
    }
    if (key == "enabled") {
      enabled = value.toInt();
    }
    if (key == "soilIni") {
      soilIni = value.toInt();
    }
    if (key == "soilIni") {
      soilIni = value.toInt();
    }
  }

  // { "type": "soilRange", "relayName": "...", "enabled": true, "startHour": 0, "lightHours": 18 }
  bool configureJSON(String json) {
    jsmn_parser p;
    jsmntok_t t[14];
    char* buffer = strdup(json.c_str());
    jsmn_init(&p);
    int j = jsmn_parse(&p, buffer, strlen(buffer), t, sizeof(t)/sizeof(t[0]));
    if (j < 0) {
      free(buffer);
      return false;
    }

    if (j < 1 || t[0].type != JSMN_OBJECT) {
      free(buffer);
      return false;
    }
    String tp;
    String r;
    String e;
    String s;
    String f;
    for (int i = 1; i < j; i++) {
      if (jsoneq(buffer, &t[i], "type") == 0) {
        tp = charToString(buffer, t[i+1].start, t[i+1].end);
        tp.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "relayName") == 0) {
        r = charToString(buffer, t[i+1].start, t[i+1].end);
        r.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "enabled") == 0) {
        e = charToString(buffer, t[i+1].start, t[i+1].end);
        e.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "soilIni") == 0) {
        s = charToString(buffer, t[i+1].start, t[i+1].end);
        s.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "soilEnd") == 0) {
        f = charToString(buffer, t[i+1].start, t[i+1].end);
        f.trim();
        i++;
        continue;
      }
    }
    free(buffer);

    if (tp == "soilRange") {
      if (r.length() > 0) {
        relayName = r;
      }
      if (e.length() > 0) {
        enabled = (e == "1" || e == "true");
      }
      if (s.length() > 0) {
        soilIni = s.toInt();
      }
      if (f.length() > 0) {
        soilEnd = f.toInt();
      }
      return true;
    }
    return false;
  }

private:
  String relayName;
  bool enabled;
  int soilIni;
  int soilEnd;
};

#endif
