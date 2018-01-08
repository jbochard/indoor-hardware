#ifndef GROW_RULE_H
#define GROW_RULE_H

#include <Arduino.h>
#include <memory>
#include <jsmn.h>
#include "rule.h"
#include "clockRangeRule.h"
#include "../../utils/utils.h"

class GrowRule : public Rule {

public:
  GrowRule(std::shared_ptr<Hardware> h, std::shared_ptr<Rule> cr) : Rule(h) {
    clockRange = cr;
    enabled = false;
    vegetativeStageDays = 100;
    floweringStageDays = 100;
    vegetativeHours = 18;
    floweringHours = 12;
    startDate.year   = 2017;
    startDate.month  = 1;
    startDate.day    = 1;
    startDate.hour   = 0;
    startDate.minute = 0;
    startDate.second = 0;
  }

  String name() {
    return "growRange";
  }

  void execute() {
    if (enabled) {
      ClockValue now = hardware->readClock();
      ClockValue dateEnd = addClockValue(startDate, vegetativeStageDays + floweringStageDays);
      if (daysFrom(now) > daysFrom(dateEnd)) {
        enabled = false;
        clockRange->setProperty("enabled", "false");
        return;
      }
      dateEnd = addClockValue(startDate, vegetativeStageDays);
      if (daysFrom(now) > daysFrom(dateEnd)) {
        clockRange->setProperty("lightHours", String(floweringHours));
      } else {
        clockRange->setProperty("lightHours", String(vegetativeHours));
      }
    }
  }

  // G00100181220170101
  bool configureBuffer(String buf) {
    if (buf.charAt(0) == 'G') {
      enabled              = buf.substring(1, 2).toInt();
      vegetativeStageDays  = buf.substring(2, 5).toInt();
      floweringStageDays   = buf.substring(5, 8).toInt();
      vegetativeHours  = buf.substring(8, 10).toInt();
      floweringHours   = buf.substring(10, 12).toInt();
      startDate.year   = buf.substring(12, 16).toInt();
      startDate.month  = buf.substring(16, 18).toInt();
      startDate.day    = buf.substring(18, 20).toInt();
      startDate.hour   = 0;
      startDate.minute = 0;
      startDate.second = 0;
      return true;
    }
    return false;
  }

  String toRegister() {
    char buf[21];
    sprintf(buf, "G%d%3d%3d%2d%2d%4d%2d%2d", enabled, vegetativeStageDays, floweringStageDays, vegetativeHours, floweringHours, startDate.year, startDate.month, startDate.day);
    return charToString(buf, 0, strlen(buf));
  }

  String toJSON() {
    ClockValue now = hardware->readClock();
    int days = daysFrom(now) - daysFrom(startDate);
    days = (days >= 0)?days:0;
    return "{ \"type\": \"growRange\", \"floweringStageDays\": " + String(floweringStageDays) + ", \"vegetativeStageDays\": " + String(vegetativeStageDays) + ", \"enabled\": "+((enabled)?"true":"false")+", \"vegetativeHours\": "+String(vegetativeHours)+", \"floweringHours\": "+String(floweringHours)+", \"startDate\": \"" + getDateYMD(startDate) + "\", \"days\": "+String(days)+" }";
  }

  String getProperty(String key) {
    if (key == "enabled") {
      return ((enabled)?"true":"false");
    }
    if (key == "vegetativeStageDays") {
      return String(vegetativeStageDays);
    }
    if (key == "floweringStageDays") {
      return String(floweringStageDays);
    }
    if (key == "vegetativeHours") {
      return String(vegetativeHours);
    }
    if (key == "floweringHours") {
      return String(floweringHours);
    }
    if (key == "startDate") {
      return getDateYMD(startDate) + "T" + getTime(startDate) + "Z";
    }
    return "";
  }

  void setProperty(String key, String value) {
    if (key == "enabled") {
      enabled = (value == "1" || value == "true");
    }
    if (key == "vegetativeStageDays") {
      vegetativeStageDays = value.toInt();
    }
    if (key == "floweringStageDays") {
      floweringStageDays = value.toInt();
    }
    if (key == "vegetativeHours") {
      vegetativeHours = value.toInt();
    }
    if (key == "floweringHours") {
      floweringHours = value.toInt();
    }
    if (key == "startDate") {
      startDate = parseDateTime(value);
    }
  }

  // { "type": "growRange", "relayName": "...", "enabled": true, "startHour": 0, "lightHours": 18 }
  bool configureJSON(String json) {
    jsmn_parser p;
    jsmntok_t t[20];
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
    String e = "";
    String l = "";
    String fs = "";
    String g = "";
    String f = "";
    String s = "";
    for (int i = 1; i < j; i++) {
      if (jsoneq(buffer, &t[i], "type") == 0) {
        tp = charToString(buffer, t[i+1].start, t[i+1].end);
        tp.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "enabled") == 0) {
        e = charToString(buffer, t[i+1].start, t[i+1].end);
        e.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "vegetativeStageDays") == 0) {
        l = charToString(buffer, t[i+1].start, t[i+1].end);
        l.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "floweringStageDays") == 0) {
        fs = charToString(buffer, t[i+1].start, t[i+1].end);
        fs.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "vegetativeHours") == 0) {
        g = charToString(buffer, t[i+1].start, t[i+1].end);
        g.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "floweringHours") == 0) {
        f = charToString(buffer, t[i+1].start, t[i+1].end);
        f.trim();
        i++;
        continue;
      }
      if (jsoneq(buffer, &t[i], "startDate") == 0) {
        s = charToString(buffer, t[i+1].start, t[i+1].end);
        s.trim();
        i++;
        continue;
      }
    }
    free(buffer);

    if (tp == "growRange") {
      if (e.length() > 0) {
        bool newEnabled = (e == "1" || e == "true");
        if (!enabled && newEnabled) {
          clockRange->setProperty("enabled", "true");
          startDate = hardware->readClock();
        } else
        if (enabled && !newEnabled) {
          clockRange->setProperty("enabled", "false");
        }
        enabled = newEnabled;
      }
      if (g.length() > 0) {
        vegetativeHours = g.toInt();
      }
      if (f.length() > 0) {
        floweringHours = f.toInt();
      }
      if (l.length() > 0) {
        vegetativeStageDays = l.toInt();
      }
      if (fs.length() > 0) {
        floweringStageDays = fs.toInt();
      }
      if (s.length() > 0) {
        startDate = parseDateTime(s);
      }
      return true;
    }
    return false;
  }

private:
  std::shared_ptr<Rule> clockRange;
  ClockValue startDate;
  bool enabled;
  int vegetativeStageDays;
  int floweringStageDays;
  int floweringHours;
  int vegetativeHours;
};

#endif
