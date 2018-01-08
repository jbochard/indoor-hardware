#include "rules.h"
#include "rules/clockRangeRule.h"
#include "rules/tempRangeRule.h"
#include "rules/humRangeRule.h"
#include "rules/soilRangeRule.h"
#include "rules/growRule.h"
#include <FS.h>

#define RULES_FILE  "rulesFile.conf"
#define BUFFER_SIZE 20

Rules::Rules(std::shared_ptr<Hardware> h) : hardware(h) {
}

void Rules::begin() {
  cont = 0;
  rules[0] = std::make_shared<ClockRangeRule>(hardware);
  rules[1] = std::make_shared<TempRangeRule>(hardware);
  rules[2] = std::make_shared<HumRangeRule>(hardware);
  rules[3] = std::make_shared<SoilRangeRule>(hardware);
  rules[4] = std::make_shared<GrowRule>(hardware, rules[0]);
  readRules();
}

void Rules::configure(String json) {
  for (int i = 0; i < RULES_SIZE; i++) {
    if (rules[i]) {
      if (rules[i]->configureJSON(json)) {
        writeRules();
        break;
      }
    }
  }
}

void Rules::update() {
  if (cont == 0) {
    for (int i = 0; i < RULES_SIZE; i++) {
      if (rules[i]) {
        rules[i]->execute();
      }
    }
  }
  cont = (cont + 1) % 10;
}

String Rules::toJSON() {
  String json = "";
  for (int i = 0; i < RULES_SIZE; i++) {
    if (rules[i]) {
      if (json.length() == 0) {
        json = json + rules[i]->toJSON();
      } else {
        json = json + ", " + rules[i]->toJSON();
      }
    }
  }
  return "{ \"rules\": [" + json + "] }";
}

void Rules::readRules() {
  File file = SPIFFS.open(RULES_FILE, "r");
  if (!file) {
    return;
  }
  String buf = file.readStringUntil('\n');
  while (buf.length() > 0) {
    for (int i = 0; i < RULES_SIZE; i++) {
      if (rules[i]) {
        if (rules[i]->configureBuffer(buf)) {
          break;
        }
      }
    }
    buf = file.readStringUntil('\n');
  }
  file.close();
}

void Rules::writeRules() {
  SPIFFS.remove(RULES_FILE);
  File file = SPIFFS.open(RULES_FILE, "w");
  if (!file) {
    return;
  }

  for (int i = 0; i < RULES_SIZE; i++) {
    if (rules[i]) {
      String buf = rules[i]->toRegister();
      file.println(buf);
    }
  }
  file.close();
}
