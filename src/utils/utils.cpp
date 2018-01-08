#include "utils.h"

String charToString(char *buf, int start, int end) {
  String tmp = "";
  for (int i = start; i < end; i++) {
    tmp = tmp + buf[i];
  }
  return tmp;
}

String floatToString(float val) {
  char buf[7];
  String res = "";
  dtostrf(val ,5 ,2, buf);
  for (int i = 0; i < 7; i++) {
    if (buf[i] == 0) {
      break;
    }
    res = res + buf[i];
  }
  return res;
}

int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  for (int i = 0; i < tok->end - tok->start; i++) {
    if (*(json + tok->start + i) != *(s + i)) {
      return -1;
    }
  }
  return 0;
}

String getDateDMY(ClockValue clock) {
  char c[11];
  sprintf(c, "%02d-%02d-%04d", clock.day, clock.month, clock.year);
  return charToString(c, 0, 10);
}

String getDateYMD(ClockValue clock) {
  char c[11];
  sprintf(c, "%04d-%02d-%02d", clock.year, clock.month, clock.day);
  return charToString(c, 0, 10);
}

String getTime(ClockValue clock) {
  char c[9];
  sprintf(c, "%02d:%02d:%02d", clock.hour, clock.minute, clock.second);
  return charToString(c, 0, 8);
}

ClockValue parseDateTime(String input) {
  ClockValue result;
  return updateDateTime(input, result);
}

ClockValue updateDateTime(String input, ClockValue result) {
  int stat = 0;
  String buf = "";
  for (int i = 0; i < input.length(); i++) {
    char t = input.charAt(i);
    if (t == '-') {
      if (stat == 0) {
        result.year = buf.toInt();
        stat = 1;
        buf = "";
      } else
      if (stat == 1) {
        result.month = buf.toInt();
        stat = 2;
        buf = "";
      }
    } else
    if (t == 'T') {
      if (stat == 2) {
        result.day = buf.toInt();
        stat = 3;
        buf = "";
      }
    } else
    if (t == ':') {
      if (stat == 0) {
        stat = 3;
      }
      if (stat == 3) {
        result.hour = buf.toInt();
        buf = "";
        stat = 4;
      } else
      if (stat == 4) {
        result.minute = buf.toInt();
        buf = "";
        stat = 5;
      }
    } else {
      buf = buf + t;
    }
  }
  if (stat == 2) {
    result.day = buf.toInt();
    result.hour = 0;
    result.minute = 0;
    result.second = 0;
  } else
  if (stat == 5) {
    result.second = buf.toInt();
  }
  return result;
}

ClockValue addClockValue(ClockValue date, int days) {
  int d = daysFrom(date);
  return dateFromDays(d + days);
}

int daysFrom(ClockValue date) {
  int m = (date.month + 9) % 12;
  int y = date.year - date.month/10;
  return 365*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + ( date.day - 1 );
}

ClockValue dateFromDays(int days) {
  int y = (10000*days + 14780)/3652425;
  int ddd = days - (365 * y + y / 4 -  y / 100 + y / 400);
  if (ddd < 0) {
    y = y - 1;
    ddd = days - (365 * y + y / 4 - y / 100 + y / 400);
  }
  int mi = (100 * ddd + 52) / 3060;
  int mm = (mi + 2) % 12 + 1;
  y = y + (mi + 2) / 12;
  int dd = ddd - (mi * 306 + 5) / 10 + 1;

  ClockValue result;
  result.year = y;
  result.month = mm;
  result.day = dd;
  result.hour = 0;
  result.minute = 0;
  result.second = 0;
  return result;
}
