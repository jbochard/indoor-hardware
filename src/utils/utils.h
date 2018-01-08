#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <jsmn.h>
#include "hardware_struct.h"

String charToString(char *buf, int start, int end);

String intToString(int val);

int stringToInt(char *buf, int ini, int length);

String floatToString(float val);

int jsoneq(const char *json, jsmntok_t *tok, const char *s);

String getDateDMY(ClockValue clock);

String getDateYMD(ClockValue clock);

String getTime(ClockValue clock);

ClockValue parseDateTime(String input);

ClockValue updateDateTime(String input, ClockValue val);

ClockValue addClockValue(ClockValue date, int days);

int daysFrom(ClockValue date);

ClockValue dateFromDays(int days);

#endif
