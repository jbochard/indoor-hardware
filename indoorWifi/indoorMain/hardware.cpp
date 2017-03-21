
#include "hardware.h"
#include "jsonFS.h"
#include <Wire.h>
#include <DHT.h>

#define countof(a) (sizeof(a) / sizeof(a[0]))
#define HARDWARE_CONF   "/hardware_conf.json"
#define LENGTH_PORT 7
#define DHT22_PIN   2
#define RELAY_1_PIN 14
#define RELAY_2_PIN 12
#define RELAY_3_PIN 13
#define RELAY_4_PIN 15
#define SOIL_MOISTURE_PIN 16
#define HARDWARE_FILE_NAME    "/hardware_conf.json"

struct HPORT {
  String  name;
  String  type;  
  int     port;
  bool    enable;
  bool    manual;
  float  value;
};

RtcDS3231<TwoWire> rtc(Wire);
DHT dht(DHT22_PIN, DHT22);
int hwd_state = 0;

HPORT ports[] = {{ "Temperatura", "DHT22_TEMP", DHT22_PIN, false, false, 0.0}, { "Humedad", "DHT22_HUM", DHT22_PIN, false, false, 0.0}, { "Relay1", "RELAY", RELAY_1_PIN, false, false, LOW}, { "Relay2", "RELAY", RELAY_2_PIN, false, false, LOW}, { "Relay3", "RELAY", RELAY_3_PIN, false, false, LOW}, { "Relay4", "RELAY", RELAY_4_PIN, false, false, LOW}, { "SoilMoisture1", "SOIL_MOISTURE", SOIL_MOISTURE_PIN, false, false, LOW}};

void readValue(int idx);
bool loadHardwareConfig();
bool saveHardwareConfig();
String toJson(int idx, bool _config);

void configHardware() {
  // Inicializa reloj.
  rtc.Begin();
  
  //Lee configuracion de archivo
  bool load = loadHardwareConfig();
  
  for (int i = 0; i < LENGTH_PORT; i++) {
    if (ports[i].type == "DHT22_TEMP") {
      dht.begin();
    }
    if (ports[i].type == "DHT22_HUM") {
      dht.begin();
    }
    if (ports[i].type == "RELAY") {
      pinMode(ports[i].port, OUTPUT);
      digitalWrite(ports[i].port, (ports[i].value==1.0)?HIGH:LOW);    
    }    
  }
  if (! load) {
    saveHardwareConfig();
  }
}

bool loadHardwareConfig() {
  bool update = false;
  String body = readFile(HARDWARE_CONF);
  if (body.indexOf("error" < 0)) {
    JsonArray & arr = jsonArray(body);
    for (int i = 0; i < LENGTH_PORT; i++) {
      if (i < arr.size()) {
        JsonVariant var = arr.get<JsonVariant>(i);
        JsonObject & obj = var.as<JsonObject>();
        ports[i].name = obj.get<String>("name");
        ports[i].type = obj.get<String>("type");
        ports[i].port = obj.get<int>("port");
        ports[i].enable = obj.get<bool>("enable");
        ports[i].manual = obj.get<bool>("manual");        
      } else {
        update = true;
      }
    }
    if (update) {
      saveHardwareConfig();
    }
    return true;
  }
  return false;
}

String jsonHardware(bool _config) {
  String jsonBuff = "";
  for (int i = 0; i < LENGTH_PORT; i++) {
    if (i > 0) {
      jsonBuff = jsonBuff + ",";
    }
    jsonBuff = jsonBuff + toJson(i, _config);
  }  
  return "["+jsonBuff+"]"; 
}

String toJson(int i, bool _config) {
  String json = "\"name\": \""+ports[i].name+"\",\"type\": \""+ports[i].type+"\",\"port\": "+String(ports[i].port)+",\"enable\": "+(ports[i].enable?"true":"false")+",\"manual\": "+(ports[i].manual?"true":"false");
  if (! _config) {
    json = json + ", \"value\": "+String(ports[i].value);
  }
 return "{ "+json+" }" ;
}

void resetHardware() {
  deleteFile(HARDWARE_CONF);
}

bool saveHardwareConfig() {
  String json = jsonHardware(true);
  writeFile(HARDWARE_CONF, json);
  return true;  
}

bool isSensor(String name) {
 for (int i = 0; i < LENGTH_PORT; i++) {
    if (ports[i].name == name) {
      return true;      
    }
  }
  return false;
}

void readHardware() {
  if (hwd_state == 0) {
    RtcDateTime currentTime = rtc.GetDateTime();
    for (int i = 0; i < LENGTH_PORT; i++) {
      if (ports[i].enable) {
        readValue(i);
      }
    }
  }
  hwd_state = (hwd_state + 1) % 10;
}

float getHardwareValue(String name) {
  for (int i = 0; i < LENGTH_PORT; i++) {
    if (ports[i].name == name) {
      if (ports[i].enable) {
        return ports[i].value;
      } else {
        return SENSOR_DISABLED;
      }
    }
  }  
  return SENSOR_WRONG;  
}

float setHardwareValue(String name, String val, bool _manual) {
  for (int i = 0; i < LENGTH_PORT; i++) {
    if (ports[i].name == name) {
      if (ports[i].enable) {
        if (ports[i].type == "RELAY" && (_manual == ports[i].manual)) {
          ports[i].value = (val=="HIGH")?HIGH:LOW;
          digitalWrite(ports[i].port, (val=="HIGH")?HIGH:LOW);
        }
        return ports[i].value;
      } else {
        return SENSOR_DISABLED;
      }
    }
  }  
  return SENSOR_WRONG;   
}

float enableHardware(String _name, bool val) {
  for (int i = 0; i < LENGTH_PORT; i++) {
    if (ports[i].name == _name) {
      if (ports[i].enable != val) {
        ports[i].enable = val;
        saveHardwareConfig();
      }
      return 0.0;
    }
  }  
  return SENSOR_WRONG;  
}


float manualHardware(String _name, bool val) {
  for (int i = 0; i < LENGTH_PORT; i++) {
    if (ports[i].name == _name) {
      if (ports[i].manual != val) {
        ports[i].manual = val;
        saveHardwareConfig();
      }
      return 0.0;
    }
  }  
  return SENSOR_WRONG;  
}

void readValue(int idx) {
  if (ports[idx].type == "DHT22_TEMP") {
    float t = dht.readTemperature();
    if (! isnan(t)) {
      ports[idx].value = t;
    }
  }
  if (ports[idx].type == "DHT22_HUM") {
    float t = dht.readHumidity();
    if (! isnan(t)) {
      ports[idx].value = t;
    } else {
      dht.begin();
    }
  }  
  if (ports[idx].type == "SOIL_MOISTURE") {
    float t = digitalRead(ports[idx].port);
    ports[idx].value = t;
  }   
}

String loadClock() {
  RtcDateTime dt = rtc.GetDateTime(); 
  return "{ \"year\": "+String(dt.Year())+",\"month\": "+String(dt.Month())+",\"day\": "+String(dt.Day())+",\"hour\": "+String(dt.Hour())+",\"minute\": "+String(dt.Minute())+",\"second\": "+String(dt.Second())+"}"; 
}

RtcDateTime getClock() {
  return rtc.GetDateTime();
}

bool setClock(String str_year, String str_month, String str_day, String str_hour, String str_minute, String str_second) {
  RtcDateTime dt = rtc.GetDateTime(); 
  int year = (str_year.length() > 0)?str_year.toInt():dt.Year();
  int month = (str_month.length() > 0)?str_month.toInt():dt.Month();
  int day = (str_day.length() > 0)?str_day.toInt():dt.Day();
  int hour = (str_hour.length() > 0)?str_hour.toInt():dt.Hour();
  int minute = (str_minute.length() > 0)?str_minute.toInt():dt.Minute();
  int second = (str_second.length() > 0)?str_second.toInt():dt.Second();
  RtcDateTime newDt = RtcDateTime(year, month, day, hour, minute, second);
  rtc.SetDateTime(newDt);
  return true;
}

