#include "jsonFS.h"
#include <FS.h>

#define JSON_BUFFER_SIZE 1200

bool initFS() {
  bool res = SPIFFS.begin();
  return res;
}

String readFile(String fileName) {
  File configFile = SPIFFS.open(fileName, "r");
  if (!configFile) {
    return "{ \"error\": \""+fileName+" no encontrado.\" }";
  }

  size_t file_size = configFile.size();
  if (file_size > JSON_BUFFER_SIZE) {
    Serial.println("Error: tamaño mayor a "+String(JSON_BUFFER_SIZE)+" - tamaño: "+String(file_size));
    return "{ \"error\": \"El tamaño de "+fileName+" es mayor a "+String(JSON_BUFFER_SIZE)+"\" }";
  }
  char buf[JSON_BUFFER_SIZE];
  configFile.readBytes(buf, JSON_BUFFER_SIZE);
  configFile.close();

  return String(buf);
}

bool writeFile(String fileName, String body) {
  SPIFFS.remove(fileName);
  File f = SPIFFS.open(fileName, "w");
  if (! f) {
    return false;
  }
  f.print(body);
  f.close();
  return true;
}

void deleteFile(String fileName) {
  SPIFFS.remove(fileName);  
}

JsonObject & jsonObject(String body) {
  if (body.length() > JSON_BUFFER_SIZE) {
    Serial.println("Error: tamaño mayor a "+String(JSON_BUFFER_SIZE)+" - tamaño: "+String(body.length()));
    return jsonError("El tamaño del json es mayor a "+String(JSON_BUFFER_SIZE));
  }
  
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  char buf[JSON_BUFFER_SIZE];
  body.toCharArray(buf, JSON_BUFFER_SIZE);

  JsonObject& json = jsonBuffer.parseObject(buf);
  if (!json.success()) {
    return jsonError("Error al parsear json.");
  }
  return json;
}

JsonArray & jsonArray(String body) {
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  if (body.length() > JSON_BUFFER_SIZE) {
    return jsonBuffer.createArray();
  }
  char buf[JSON_BUFFER_SIZE];
  body.toCharArray(buf, JSON_BUFFER_SIZE);
  return jsonBuffer.parseArray(buf);
}

JsonObject & jsonError(String msg) {
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  if (msg.length() > 60) {
    msg = "Mensaje de error demasiado largo.";
  }
  char buf[JSON_BUFFER_SIZE];
  String json = "{\"error\":\""+msg+"\"}";
  json.toCharArray(buf, JSON_BUFFER_SIZE);
  JsonObject& root = jsonBuffer.parseObject(buf);
  return root;
}

