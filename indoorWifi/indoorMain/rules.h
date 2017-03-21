#ifndef H_RULES
#define H_RULES

#include <Arduino.h>

 bool evaluateRules();
 
 String listJsonRules();

 void insertRule(String rule);

 void deleteRules();
 
 void deleteRule(int idx);
 
#endif

