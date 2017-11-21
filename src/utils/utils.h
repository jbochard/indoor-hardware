#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <jsmn.h>
#include "hardware_struct.h"

String charToString(char *buf, int start, int end);

String intToString(int val);

int stringToInt(char *buf, int ini, int length);

int jsoneq(const char *json, jsmntok_t *tok, const char *s);

String getDate(ClockValue clock);

String getTime(ClockValue clock);

#endif
