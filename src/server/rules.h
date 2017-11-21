#ifndef RULES_H
#define RULES_H

#include <Arduino.h>
#include <memory>
#include "hardware.h"
#include "rules/rule.h"

#define RULES_SIZE  4

class Rules {

private:
  short cont = 0;

  std::shared_ptr<Hardware> hardware;
  std::unique_ptr<Rule>     rules[RULES_SIZE];

  void readRules();

  void writeRules();

public:
  Rules(std::shared_ptr<Hardware> h);

  void begin();

  void configure(String json);

  String toJSON();

  void update();

};

#endif
