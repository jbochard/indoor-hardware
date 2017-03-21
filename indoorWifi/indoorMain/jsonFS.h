#ifndef H_JSON_FS
#define H_JSON_FS

#include <ArduinoJson.h>

bool initFS();

String readFile(String fileName);

bool writeFile(String fileName, String body);

void deleteFile(String fileName);

JsonObject & jsonObject(String body);

JsonArray & jsonArray(String body);

JsonObject & jsonError(String msg);

#endif

