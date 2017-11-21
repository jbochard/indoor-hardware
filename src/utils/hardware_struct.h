#ifndef HARDWARE_STRUCT_H
#define HARDWARE_STRUCT_H

#include <Arduino.h>

struct ClockValue {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
};

struct HSENSOR {
  String  name;
  int     pin;
  float   value;
  float   buf;
};

struct HSWITCH {
  String  name;
  int     pin;
  bool    manual;
  bool    state;
};

#endif
