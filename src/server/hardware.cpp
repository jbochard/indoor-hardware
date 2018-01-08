#include "hardware.h"
#include "../utils/utils.h"
#include <jsmn.h>
#include <FS.h>
//#include <Wire.h>
#include <RTClib.h>
#include <DHT.h>

#define DHT22_PIN         D3
#define SOIL_MOISTURE_PIN A0
#define RELAY_1_PIN       D5
#define RELAY_2_PIN       D6
#define RELAY_3_PIN       D7
#define RELAY_4_PIN       D8

#define HARDWARE_FILE "hardware.conf"
#define SMOOTH        0.3
#define SENSOR_SIZE   3
#define SWITCHS_SIZE  4

RTC_DS3231 rtc;
DHT dht;

HSENSOR sensors[] = {{ "Temperatura", DHT22_PIN, 0.0, 0.0}, { "Humedad", DHT22_PIN, 0.0, 0.0}, { "SoilMoisture", SOIL_MOISTURE_PIN, 0.0, 0.0}};
HSWITCH switchs[] = {{ "Relay1", RELAY_1_PIN, false, false}, { "Relay2", RELAY_2_PIN, false, false}, { "Relay3", RELAY_3_PIN, false, false}, { "Relay4", RELAY_4_PIN, false, false}};

void Hardware::begin() {
  cont = 0;
  dht.setup(DHT22_PIN, DHT::DHT22);
  rtc.begin();
  for (int i = 0; i < SENSOR_SIZE; i++) {
    pinMode(sensors[i].pin, INPUT);
  }
  for (int i = 0; i < SWITCHS_SIZE; i++) {
    pinMode(switchs[i].pin, OUTPUT);
    digitalWrite(switchs[i].pin, LOW);
  }
  readConfiguration();
}

int Hardware::getSensorNumber() {
  return SENSOR_SIZE;
}

String Hardware::getSensorName(int idx) {
  if (idx >= 0 && idx < SENSOR_SIZE) {
    return sensors[idx].name;
  }
  return "";
}

HSENSOR Hardware::readSensor(String sensorName) {
  if (sensorName == "Temperatura") {
    return sensors[0];
  }
  if (sensorName == "Humedad") {
    return sensors[1];
  }
  if (sensorName == "SoilMoisture") {
    return sensors[2];
  }
  return { "ERROR", -1, -1 };
}

HSENSOR Hardware::readSensor(int idx) {
  if (idx < SENSOR_SIZE && idx >= 0) {
    return sensors[idx];
  }
  return { "ERROR", -1, -1 };
}

String Hardware::printSensor(String sensorName) {
  char tmp[8];
  if (sensorName == "Temperatura") {
    if (isnan(sensors[0].value)) {
      return "Tmp: ERROR     ";
    }
    return "Tmp: " + String(dtostrf(sensors[0].value, 4, 2, tmp)) + "   ";
  }
  if (sensorName == "Humedad") {
    if (isnan(sensors[1].value)) {
      return "Hum: ERROR     ";
    }
    return "Hum: " + String(dtostrf(sensors[1].value, 4, 2, tmp)) + "   ";
  }
  if (sensorName == "SoilMoisture") {
    if (isnan(sensors[2].value)) {
      return "Hdr: ERROR     ";
    }
    return "Hdr: " + String(dtostrf(sensors[2].value, 5, 2, tmp)) + "   ";
  }
  return "Fld: ERROR     ";
}

String Hardware::printSwitch(String sensorName) {
  int idx = -1;
  if (sensorName == "Relay1") {
    idx = 0;
  }
  if (sensorName == "Relay2") {
    idx = 1;
  }
  if (sensorName == "Relay3") {
    idx = 2;
  }
  if (sensorName == "Relay4") {
    idx = 3;
  }
  if (idx >= 0) {
    return "Rl" + String(idx+1) + ": " + String((switchs[idx].state)?" ON":"OFF") + " - " + String((switchs[idx].manual)?"MANUAL":"  AUTO");
  } else {
    return "ERROR";
  }
}

int Hardware::getSwitchNumber() {
  return SWITCHS_SIZE;
}

String Hardware::getSwitchName(int idx) {
  if (idx >= 0 && idx < SWITCHS_SIZE) {
    return switchs[idx].name;
  }
  return "";
}

HSWITCH Hardware::readSwitch(String switchName) {
  int idx = switchIdx(switchName);
  if (idx < 0) {
    return { "ERROR", -1, false, false };
  }
  return switchs[idx];
}

HSWITCH Hardware::readSwitch(int idx) {
  if (idx >= 0 && idx < SWITCHS_SIZE) {
    return switchs[idx];
  }
  return { "ERROR", -1, false, false };
}

HSWITCH Hardware::writeSwitch(String name, String key, bool value, bool automatic) {
  int idx = switchIdx(name);
  if (idx >= 0 && idx < SWITCHS_SIZE) {
    if (key == "manual") {
      switchs[idx].manual = value;
    }
    if (key == "value") {
      if (automatic && !switchs[idx].manual || !automatic && switchs[idx].manual) {
        switchs[idx].state = value;
        if (switchs[idx].state) {
          digitalWrite(switchs[idx].pin, HIGH);
        } else {
          digitalWrite(switchs[idx].pin, LOW);
        }
      }
    }
    return switchs[idx];
  }
  return { "ERROR", -1, false, false };
}

HSWITCH Hardware::switchUpdate(String name, String body) {
  int idx = switchIdx(name);
  if (idx < 0) {
    return { "ERROR", -1, false, false };
  }
  jsmn_parser p;
  jsmntok_t t[8]; /* We expect no more than 128 tokens */
  char* buffer = strdup(body.c_str());

  jsmn_init(&p);
  int r = jsmn_parse(&p, buffer, strlen(buffer), t, sizeof(t)/sizeof(t[0]));
  if (r < 0) {
    free(buffer);
    return { "ERROR", -1, false, false };
  }

	if (r < 1 || t[0].type != JSMN_OBJECT) {
    free(buffer);
    return { "ERROR", -1, false, false };
	}

  String sManual= "";
  String sState = "";
	/* Loop over all keys of the root object */
	for (int i = 1; i < r; i++) {
		if (jsoneq(buffer, &t[i], "manual") == 0) {
      sManual = charToString(buffer, t[i+1].start, t[i+1].end);
      sManual.trim();
      i++;
      continue;
		}
    if (jsoneq(buffer, &t[i], "value") == 0) {
      sState = charToString(buffer, t[i+1].start, t[i+1].end);
      sState.trim();
      i++;
      continue;
		}
	}
  free(buffer);

  HSWITCH result;
  if (sManual.length() > 0) {
    result = writeSwitch(name, "manual", (sManual == "true" || sManual == "1"), false);
  }
  if (sState.length() > 0) {
    result = writeSwitch(name, "value", (sState == "true" || sState == "1"), false);
  }
  writeConfiguration();
  return result;
}

void Hardware::mean(float val, int idx) {
  if (isnan(val)) {
    sensors[idx].value = val;
  } else {
    sensors[idx].buf = sensors[idx].value;
    sensors[idx].value = SMOOTH * val + (1-SMOOTH) * sensors[idx].buf;
  }
}

void Hardware::update() {
  if (cont == 0) {
    float tmp = dht.getTemperature();
    if (isnan(tmp)) {
      Serial.println(dht.getStatusString());
    }
    mean(tmp, 0);
    tmp = dht.getHumidity();
    if (isnan(tmp)) {
      Serial.println(dht.getStatusString());
    }
    mean(tmp, 1);
    mean(map(analogRead(SOIL_MOISTURE_PIN), 0, 1024, 100, 0), 2);
  }
  cont = (cont + 1) % 20;
}

ClockValue Hardware::readClock() {
  ClockValue res;
  DateTime now = rtc.now();
  res.year = now.year();
  res.month = now.month();
  res.day = now.day();
  res.hour = now.hour();
  res.minute = now.minute();
  res.second = now.second();
  return res;
}

ClockValue Hardware::writeClock(String body) {
  ClockValue res = readClock();
  jsmn_parser p;
  jsmntok_t t[6]; /* We expect no more than 128 tokens */
  char* buffer = strdup(body.c_str());

  jsmn_init(&p);
  int r = jsmn_parse(&p, buffer, strlen(buffer), t, sizeof(t)/sizeof(t[0]));
  if (r < 0) {
    free(buffer);
    return res;
  }

	if (r < 1 || t[0].type != JSMN_OBJECT) {
    free(buffer);
    return res;
	}

  String sDate= "";
  String sTime = "";
	/* Loop over all keys of the root object */
	for (int i = 1; i < r; i++) {
		if (jsoneq(buffer, &t[i], "date") == 0) {
      sDate = charToString(buffer, t[i+1].start, t[i+1].end);
      sDate.trim();
      i++;
      continue;
		}
    if (jsoneq(buffer, &t[i], "time") == 0) {
      sTime = charToString(buffer, t[i+1].start, t[i+1].end);
      sTime.trim();
      i++;
      continue;
		}
	}
  free(buffer);

  if (sDate.length() > 0) {
    res = updateDateTime(sDate, res);
  }
  if (sTime.length() > 0) {
    res = updateDateTime(sTime, res);
  }
  Serial.println(getDateYMD(res));
  DateTime dt = DateTime(res.year, res.month, res.day, res.hour, res.minute, res.second);
  rtc.adjust(dt);

  return res;
}

int Hardware::str2date(String d, int def) {
  if (d != NULL && d.length() > 0) {
    bool match = true;
    for (int i = 0; i < d.length(); i++) {
      match = match && d.charAt(i) >= 48 && d.charAt(i) <= 57;
    }
    if (match) {
      return d.toInt();
    }
  }
  return def;
}

int Hardware::switchIdx(String switchName) {
  switchName.replace("Relay", "");
  int idx = switchName.toInt();
  if (idx == 0 || idx > SWITCHS_SIZE) {
    return -1;
  }
  return idx-1;
}

void Hardware::readConfiguration() {
  File file = SPIFFS.open(HARDWARE_FILE, "r");
  if (!file) {
    return;
  }
  size_t size = file.size();
  std::unique_ptr<char[]> buf(new char[size]);

  file.readBytes(buf.get(), size);
  file.close();

  for (int i = 0; i < SWITCHS_SIZE; i++) {
    if (i < size) {
      switchs[i].manual = (int)buf[i];
      digitalWrite(switchs[i].pin, switchs[i].state?HIGH:LOW);
    }
  }
}

void Hardware::writeConfiguration() {
  SPIFFS.remove(HARDWARE_FILE);
  File f = SPIFFS.open(HARDWARE_FILE, "w");
  if (f) {
    String buf = "";
    for (int i = 0; i < SWITCHS_SIZE; i++) {
      buf = buf + ""+char(switchs[i].manual);
    }
    f.print(buf);
    f.close();
  }
}
