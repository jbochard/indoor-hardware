#ifndef RULE_H
#define RULE_H

#include <Arduino.h>
#include <memory>


class Rule {

public:
  Rule(std::shared_ptr<Hardware> h) : hardware(h) {
  }

  virtual String name() = 0;

  virtual void execute() = 0;

  virtual String getProperty(String key) = 0;
  
  virtual void setProperty(String key, String value) = 0;

  virtual bool configureBuffer(String buf) = 0;

  // { "type": "...", ... }
  virtual bool configureJSON(String json) = 0;

  virtual String toRegister() = 0;

  virtual String toJSON() = 0;

  protected:
    std::shared_ptr<Hardware> hardware;
};

#endif
