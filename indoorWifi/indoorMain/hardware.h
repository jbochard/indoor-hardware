#ifndef H_HARDWARE
#define H_HARDWARE

#include <Arduino.h>
#include <RtcDS3231.h> 

#define HARDWARE_OK     0
#define HARDWARE_ERROR  1
#define SENSOR_DISABLED   -1.0
#define SENSOR_WRONG      -2.0

void configHardware();

void readHardware();

bool isSensor(String name);

String jsonHardware(bool structure);

float getHardwareValue(String name);

float setHardwareValue(String name, String val, bool _manual);

float enableHardware(String name, bool val);

float manualHardware(String name, bool val);

String loadClock();

RtcDateTime getClock();

void resetHardware();

bool setClock(String year, String month, String day, String hour, String minute, String second);

#endif

