#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include "../utils/hardware_struct.h"

class Hardware {

public:
  void begin();

  int getSensorNumber();

  String getSensorName(int idx);

  HSENSOR readSensor(String sensorName);

  HSENSOR readSensor(int idx);

  String printSensor(String sensorName);

  int getSwitchNumber();

  String getSwitchName(int idx);

  HSWITCH readSwitch(String switchName);

  HSWITCH readSwitch(int idx);

  HSWITCH writeSwitch(String name, String key, bool value, bool automatic);

  String printSwitch(String sensorName);

  HSWITCH switchUpdate(String name, String body);

  ClockValue readClock();

  ClockValue writeClock(String body);

  void update();

private:
  short cont = 0;

  void mean(float val, int idx);

  int switchIdx(String switchName);

  void readConfiguration();

  void writeConfiguration();

  int str2date(String d, int def);
};

#endif
