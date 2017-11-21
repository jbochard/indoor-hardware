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

  String printSensor(String sensorName);

  int getSwitchNumber();

  String getSwitchName(int idx);

  HSWITCH readSwitch(String switchName);

  void writeSwitch(String name, String key, bool value);

  String printSwitch(String sensorName);

  int switchUpdate(String name, String body);

  ClockValue readClock();

  ClockValue writeClock(String year, String month, String day, String hour, String minute, String second);

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
