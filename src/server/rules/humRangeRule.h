#ifndef HUM_RANGE_RULE_H
#define HUM_RANGE_RULE_H

#include <Arduino.h>
#include <memory>
#include <jsmn.h>
#include "rule.h"
#include "../../utils/utils.h"

class HumRangeRule : public Rule {

public:
  HumRangeRule(std::shared_ptr<Hardware> h) : Rule(h) {
    relayName = "Relay1";
    enabled = false;
    humIni = 0.0;
    humEnd = 0.0;  
  }

  String name() {
    return "humRange";
  }

  void execute() {
    if (enabled) {
      bool apply = false;
      HSENSOR sensor = hardware->readSensor("Humedad");
      if (humIni == 0) {
        apply = sensor.value <= humEnd;
      }
      if (humEnd == 0) {
        apply = sensor.value >= humIni;
      }
      if (humIni > 0 && humEnd > 0) {
        apply = sensor.value >= humIni && sensor.value <= humEnd;
      }
      hardware->writeSwitch(relayName, "state", apply);
    }
  }

  // H020.0025.00Relay1
  bool configureBuffer(String buf) {
    if (buf.charAt(0) == 'H') {
      enabled    = buf.substring(1, 2).toInt();
      humIni     = buf.substring(2, 7).toFloat();
      humEnd     = buf.substring(7, 12).toFloat();
      relayName  = buf.substring(12, 18);
      return true;
    }
    return false;
  }

  String toRegister() {
    char buf[18];
    char sHumIni[6];
    dtostrf(humIni ,5 ,2, sHumIni);
    char sHumEnd[6];
    dtostrf(humEnd ,5 ,2, sHumEnd);

    sprintf(buf, "H%d%5s%5s%5s", enabled, sHumIni, sHumEnd, relayName.c_str());
    return charToString(buf, 0, strlen(buf));
  }

  String toJSON() {
    return "{ \"type\": \"humRange\",  \"relayName\": \""+relayName+"\", \"enabled\": "+((enabled)?"true":"false")+", \"humIni\": "+String(humIni)+", \"humEnd\": "+String(humEnd)+" }";
  }

  // { "type": "humRange", "relayName": "...", "enabled": true, "startHour": 0, "lightHours": 18 }
  bool configureJSON(String json) {
    jsmn_parser p;
    jsmntok_t t[11];
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
      if (jsoneq(buffer, &t[i], "humIni") == 0) {
        s = charToString(buffer, t[i+1].start, t[i+1].end);
        s.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "humEnd") == 0) {
        f = charToString(buffer, t[i+1].start, t[i+1].end);
        f.trim();
        i++;
        continue;
      }
    }
    free(buffer);

    if (tp == "humRange") {
      if (r.length() > 0) {
        relayName = r;
      }
      if (e.length() > 0) {
        enabled = (e == "1" || e == "true");
      }
      if (s.length() > 0) {
        humIni = s.toFloat();
      }
      if (f.length() > 0) {
        humEnd = f.toFloat();
      }
      return true;
    }
    return false;
  }

private:
  String relayName;
  bool enabled;
  float humIni;
  float humEnd;
};

#endif
