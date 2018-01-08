#ifndef TEMP_RANGE_RULE_H
#define TEMP_RANGE_RULE_H

#include <Arduino.h>
#include <memory>
#include <jsmn.h>
#include "rule.h"
#include "../../utils/utils.h"

class TempRangeRule : public Rule {

public:
  TempRangeRule(std::shared_ptr<Hardware> h) : Rule(h) {
    relayName = "Relay1";
    enabled = false;
    tempIni = 0.0;
    tempEnd = 0.0;
  }

  String name() {
    return "tempRange";
  }
  void execute() {
    if (enabled) {
      bool apply = false;
      HSENSOR sensor = hardware->readSensor("Temperatura");
      if (tempIni == 0) {
        apply = sensor.value <= tempEnd;
      }
      if (tempEnd == 0) {
        apply = sensor.value >= tempIni;
      }
      if (tempIni > 0 && tempEnd > 0) {
        apply = sensor.value >= tempIni && sensor.value <= tempEnd;
      }
      hardware->writeSwitch(relayName, "value", apply, true);
    }
  }

  // T020.0025.00Relay1
  bool configureBuffer(String buf) {
    if (buf.charAt(0) == 'T') {
      enabled    = buf.substring(1, 2).toInt();
      tempIni    = buf.substring(2, 7).toFloat();
      tempEnd    = buf.substring(7, 12).toFloat();
      relayName  = buf.substring(12, 18);
      return true;
    }
    return false;
  }

  String toRegister() {
    char buf[18];
    char sTempIni[6];
    dtostrf(tempIni ,5 ,2, sTempIni);
    char sTempEnd[6];
    dtostrf(tempEnd ,5 ,2, sTempEnd);

    sprintf(buf, "T%d%5s%5s%5s", enabled, sTempIni, sTempEnd, relayName.c_str());
    return charToString(buf, 0, strlen(buf));
  }

  String toJSON() {
    return "{ \"type\": \"tempRange\",  \"relayName\": \""+relayName+"\", \"enabled\": "+((enabled)?"true":"false")+", \"tempIni\": "+String(tempIni)+", \"tempEnd\": "+String(tempEnd)+" }";
  }

  String getProperty(String key) {
    if (key == "enabled") {
      return ((enabled)?"true":"false");
    }
    if (key == "relayName") {
      return relayName;
    }
    if (key == "tempIni") {
      return floatToString(tempIni);
    }
    if (key == "tempEnd") {
      return floatToString(tempEnd);
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
    if (key == "tempIni") {
      tempIni = value.toFloat();
    }
    if (key == "tempEnd") {
      tempEnd = value.toFloat();
    }
  }

  // { "type": "tempRange", "relayName": "...", "enabled": true, "startHour": 0, "lightHours": 18 }
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
      if (jsoneq(buffer, &t[i], "tempIni") == 0) {
        s = charToString(buffer, t[i+1].start, t[i+1].end);
        s.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "tempEnd") == 0) {
        f = charToString(buffer, t[i+1].start, t[i+1].end);
        f.trim();
        i++;
        continue;
      }
    }
    free(buffer);

    if (tp == "tempRange") {
      if (r.length() > 0) {
        relayName = r;
      }
      if (e.length() > 0) {
        enabled = (e == "1" || e == "true");
      }
      if (s.length() > 0) {
        tempIni = s.toFloat();
      }
      if (f.length() > 0) {
        tempEnd = f.toFloat();
      }
      return true;
    }
    return false;
  }

private:
  String relayName;
  bool enabled;
  float tempIni;
  float tempEnd;
};

#endif
