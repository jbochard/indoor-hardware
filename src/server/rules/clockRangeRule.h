#ifndef CLOCK_RANGE_RULE_H
#define CLOCK_RANGE_RULE_H

#include <Arduino.h>
#include <memory>
#include <jsmn.h>
#include "rule.h"
#include "../../utils/utils.h"

class ClockRangeRule : public Rule {

public:
  ClockRangeRule(std::shared_ptr<Hardware> h) : Rule(h) {
    relayName = "Relay1";
    enabled = false;
    startHour = 0;
    lightHours = 18;   
  }

  String name() {
    return "clockRange";
  }

  void execute() {
    if (enabled) {
      bool apply = false;
      ClockValue clock = hardware->readClock();
      int endHour = (startHour + lightHours) % 24;
      if (endHour < startHour) {
        apply = clock.hour >= startHour && clock.hour <= 23 || clock.hour >= 0 && clock.hour <= endHour;
      } else {
        apply = clock.hour >= startHour && clock.hour <= endHour;
      }
      hardware->writeSwitch(relayName, "state", apply);
    }
  }

  // C0 018Relay1
  bool configureBuffer(String buf) {
    if (buf.charAt(0) == 'C') {
      enabled    = buf.substring(1, 2).toInt();
      startHour  = buf.substring(2, 4).toInt();
      lightHours = buf.substring(4, 6).toInt();
      relayName  = buf.substring(6, 12);
      return true;
    }
    return false;
  }

  String toRegister() {
    char buf[12];
    sprintf(buf, "C%d%2d%2d%5s", enabled, startHour, lightHours, relayName.c_str());
    return charToString(buf, 0, strlen(buf));
  }

  String toJSON() {
    return "{ \"type\": \"clockRange\",  \"relayName\": \""+relayName+"\", \"enabled\": "+((enabled)?"true":"false")+", \"startHour\": "+String(startHour)+", \"lightHours\": "+String(lightHours)+" }";
  }

  // { "type": "clockRange", "relayName": "...", "enabled": true, "startHour": 0, "lightHours": 18 }
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
    String tp = "";
    String r = "";
    String e = "";
    String s = "";
    String l = "";
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
      if (jsoneq(buffer, &t[i], "startHour") == 0) {
        s = charToString(buffer, t[i+1].start, t[i+1].end);
        s.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "lightHours") == 0) {
        l = charToString(buffer, t[i+1].start, t[i+1].end);
        l.trim();
        i++;
        continue;
      }
    }
    free(buffer);

    if (tp == "clockRange") {
      if (r.length() > 0) {
        relayName = r;
      }
      if (e.length() > 0) {
        enabled = (e == "1" || e == "true");
      }
      if (s.length() > 0) {
        startHour = s.toInt();
      }
      if (l.length() > 0) {
        lightHours = l.toInt();
      }
      return true;
    }
    return false;
  }

private:
  String relayName;
  bool enabled;
  int startHour;
  int lightHours;
};

#endif
