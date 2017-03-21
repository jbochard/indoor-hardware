#include <QList.h>
#include "rules.h"
#include "hardware.h"
#include "jsonFS.h"

#define RULE_FILE_NAME  "/rules.json"

/*
 * rules = rule 
 *       | rule "\n" rules
 * rule =   "if" conditions "then" statements
 *        | "if" conditions "then" statements "else" statements
 * conditions =   andConditions
 *              | andConditions "or" conditions
 *              | "(" conditions ")"
 * andConditions =    negCondition
 *                  | negCondition "and" andConditions
 * negCondition =   condition
 *                | "!" condition
 * condition =    term ("=" | ">" | ">=" | "<" | "<=") term
 *              | term "in "(" values ")"
 * term =   sensor 
 *        | ("anio_actual" | "mes_actual" | "dia_actual" | "hora_actual" | "minuto_actual" | "segundo_actual")
 *        | value
 * sensor = _READ_VALUE(sensor)
 * values =     value
 *            | value "," values
 * value = [0..9]+
 * 
 * statements =   statement
 *              | statement ";" statements
 * statement =    "on" "(" sensor ")"
 *              | "off" "(" sensor ")"
 */

bool error = false;

bool evalRule(String rule);
bool evalConditions(String conditions);
bool evalAndConditions(String conditions);
bool evalNotCondition(String condition);
bool evalCondition(String condition);
float evalTerm(String term);
void evalValues(String values, QList<float> & result);
bool evalStatements(String statements);
bool evalStatement(String statement);
bool split(String input, String sep, String (& result) [2]);

String listJsonRules() {
  String json = readFile(RULE_FILE_NAME);
  if (json.indexOf("error") > 0) {
    json = "[]";
    writeFile(RULE_FILE_NAME, json);
  }
  return json;
}

void insertRule(String rule) {
  String json = readFile(RULE_FILE_NAME);
  if (json.indexOf("error") > 0) {
    writeFile(RULE_FILE_NAME, "[\""+rule+"\"]");
    return;
  } 
  bool first = true;
  String newJson = "";
  JsonArray & arr = jsonArray(json);
  for (int i = 0; i < arr.size(); i++) {
    String r = arr.get<String>(i);
    if (first) {
      newJson = "\""+r+"\"";
      first = false;
    } else {
      newJson = newJson + ",\""+r+"\"";
    } 
  }
  if (first) {
    newJson = "\""+rule+"\"";
  } else {
    newJson = newJson + ",\""+rule+"\"";
  }   
  writeFile(RULE_FILE_NAME, "["+newJson+"]");  
}

void deleteRules() {
  deleteFile(RULE_FILE_NAME);
}

void deleteRule(int idx) {
  String json = readFile(RULE_FILE_NAME);
  if (json.indexOf("error") > 0) {
    writeFile(RULE_FILE_NAME, "[]");
    return;
  } 
  JsonArray & arr = jsonArray(json);
  if (idx < arr.size()) {
    arr.removeAt(idx);
    
    bool first = true;
    String newJson = "";
    for (int i = 0; i < arr.size(); i++) {
      String r = arr.get<String>(i);
      if (first) {
        newJson = "\""+r+"\"";
        first = false;
      } else {
        newJson = newJson + ",\""+r+"\"";
      } 
    }
    writeFile(RULE_FILE_NAME, "["+newJson+"]");   
  } 
}

bool evaluateRules() {
  bool result = false;
  String rules = readFile(RULE_FILE_NAME);
  JsonArray & arr = jsonArray(rules);
  for (int i = 0; i < arr.size(); i++) {
    String rule = arr.get<String>(i);
    bool r = evalRule(rule);
    Serial.println(rule + " = "+ String(r));
    result = result || r;
  }
  return result;
}

// -------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------

bool evalRule(String rule) {
  String res[2];
  String conditions = "";
  String statements = "";
  String elseStatements = "";
  error = false;
  if (split(rule, "if ", res)) {
    if (split(res[0], "then ", res)) {
      conditions = res[0];
      split(res[1], "else ", res);
      statements = res[0];
      elseStatements = res[1];
      if (evalConditions(conditions) && ! error) {
        return evalStatements(statements) && error;
      } else {
        if (elseStatements.length() > 0 && ! error) {
          return evalStatements(elseStatements) && error;
        }
      }
    }
  }
  return false;  
}

bool evalConditions(String conditions) {
  if (conditions.length() == 0) {
    return false;
  }
  if (conditions.startsWith("(")) {
    return evalConditions(conditions.substring(1, conditions.length()-2));
  }
  String res[2];
  if (split(conditions, " or ", res)) {
    return evalAndConditions(res[0]) || evalConditions(res[1]);
  }
  return evalAndConditions(res[0]);
}

bool evalAndConditions(String conditions) {
  if (conditions.length() == 0) {
    return false;
  }
  String res[2];
  if (split(conditions, " and ", res)) {
    return evalNotCondition(res[0]) && evalAndConditions(res[1]);  
  }
  return evalNotCondition(res[0]);
}

bool evalNotCondition(String condition) {
  if (condition.length() == 0) {
    return false;
  }
  String cond = condition;
  if (cond.startsWith("!")) {
    cond = cond.substring(1);
    cond.trim();
    return ! evalCondition(cond);
  }
  return evalCondition(cond);  
}

bool evalCondition(String condition) {
  if (condition.length() == 0) {
    error = true;
    return false;
  }
  String res[2];
  if (condition.indexOf("=") > 0) {
    split(condition, "=", res);
    float left = evalTerm(res[0]);
    float right = evalTerm(res[1]);
    if (left < 0 || right < 0) {
      error = true;
      return false;
    }
    return left == right;    
  }
  if (condition.indexOf(">=") > 0) {
    split(condition, ">=", res);
    float left = evalTerm(res[0]);
    float right = evalTerm(res[1]);
    if (left < 0 || right < 0) {
      error = true;
      return false;
    }    
    return left >= right;    
  }  
  if (condition.indexOf("<=") > 0) {
    split(condition, "<=", res);
    float left = evalTerm(res[0]);
    float right = evalTerm(res[1]);
    if (left < 0 || right < 0) {
      error = true;
      return false;
    }
    return left <= right;    
  }  
  if (condition.indexOf(">") > 0) {
    split(condition, ">", res);
    float left = evalTerm(res[0]);
    float right = evalTerm(res[1]);
    if (left < 0 || right < 0) {
      error = true;
      return false;
    }
    return left > right;    
  }  
  if (condition.indexOf("<") > 0) {
    split(condition, "<", res);
    float left = evalTerm(res[0]);
    float right = evalTerm(res[1]);
    if (left < 0 || right < 0) {
      error = true;
      return false;
    }
    return left < right;    
  }  
  if (condition.indexOf(" in(") > 0) {
    split(condition, " in(", res);
    float left = evalTerm(res[0]);
    String values = res[1].substring(0, res[1].length()-2);
    if (left < 0 ) {
      error = true;
      return false;
    }    
    return values.indexOf(","+String(left)+",") >= 0 || values.startsWith(String(left)+",") || values.endsWith(","+String(left)) || values == String(left);    
  }  
  return false;
}

float evalTerm(String term) {
  if (isSensor(term)) {
    return getHardwareValue(term);
  }
  if (term == "anio_actual") {
    return getClock().Year();
  }
  if (term == "mes_actual") {
    return getClock().Month();
  }
  if (term == "dia_actual") {
    return getClock().Day();
  }
  if (term == "hora_actual") {
    return getClock().Hour();
  }
  if (term == "minuto_actual") {
    return getClock().Minute();
  }
  if (term == "segundo_actual") {
    return getClock().Second();
  }
  if (term == "semana_actual") {
    return getClock().DayOfWeek();
  }
  if (term == "segundos") {
    return getClock().TotalSeconds();
  }    
  return term.toFloat();
}

void evalValues(String values, QList<float> & result) {
  String res[2];
  while(values.length() > 0) {
    split(values, ",", res);
    result.push_back(res[0].toFloat());
    values = res[1];
  }
}

bool evalStatements(String statements) {
  String res[2];
  bool result = false;
  while (statements.length() > 0) {
    split(statements, ";", res);
    result = result || evalStatement(res[0]);
    statements = res[1];
  }
  return result;  
}

bool evalStatement(String statement) {
  if (statement.startsWith("on")) {
    String relay = statement.substring(3, statement.length()-2);
    setHardwareValue(relay, "HIGH", false);
    return true;
  }
  if (statement.startsWith("off")) {
    String relay = statement.substring(4, statement.length()-2);
    setHardwareValue(relay, "LOW", false);    
    return true;
  }
  return false;  
}

bool split(String input, String sep, String (& result) [2]) {
  int idx = input.indexOf(sep);
  if (idx == 0) {
    result[0] = input.substring(sep.length());
    result[0].trim();
    result[1] = "";
    return true;
  }
  if (idx > 0) {
    result[0] = input.substring(0, idx);
    result[0].trim();
    result[1] = input.substring(idx + sep.length(), input.length()-1);
    result[1].trim();
    return true;
  }
  result[0] = String(input);
  result[0].trim();
  result[1] = "";
  return false;
}

