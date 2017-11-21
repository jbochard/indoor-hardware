#include "utils.h"

String charToString(char *buf, int start, int end) {
  String tmp = "";
  for (int i = start; i < end; i++) {
    tmp = tmp + buf[i];
  }
  return tmp;
}

int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  for (int i = 0; i < tok->end - tok->start; i++) {
    if (*(json + tok->start + i) != *(s + i)) {
      return -1;
    }
  }
  return 0;
}

String getDate(ClockValue clock) {
  char c[11];
  sprintf(c, "%02d-%02d-%04d", clock.day, clock.month, clock.year);
  return charToString(c, 0, 10);
}

String getTime(ClockValue clock) {
  char c[9];
  sprintf(c, "%02d:%02d:%02d", clock.hour, clock.minute, clock.second);
  return charToString(c, 0, 8);
}
