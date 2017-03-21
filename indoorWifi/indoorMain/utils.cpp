#include "utils.h"
#include <Arduino.h>

#define BUTTON_NUM_STATE    20

LedBlink::LedBlink(int p) {
  pin = p;
  previousMillis = millis();
  ledState = LOW;
  pinMode(pin,   OUTPUT);
}

void LedBlink::blink(int interval) {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;   
    if (ledState == LOW)
      ledState = HIGH;  // Note that this switches the LED *off*
    else
      ledState = LOW;   // Note that this switches the LED *on*
    digitalWrite(pin, ledState);
  }
}

void LedBlink::on() {
  ledState = HIGH;  
  digitalWrite(pin, HIGH);  
}

void LedBlink::off() {
  ledState = LOW;  
  digitalWrite(pin, LOW);  
}


ButtonState::ButtonState(int p) {
  button_state = 0;
  pin = p;
  pinMode(pin,   INPUT);
}

int ButtonState::buttonPressed() {
  int button = digitalRead(pin);
  if (button_state == 0 && button == LOW) {
    return LOW;
  }
  if (button_state > 0 && button_state <= BUTTON_NUM_STATE && button == LOW) {
    button_state = button_state - 1;
    return LOW;
  }
  if (button_state >= 0 && button_state < BUTTON_NUM_STATE && button == HIGH) {
    button_state = button_state + 1;
    return LOW;
  }
  if (button_state == BUTTON_NUM_STATE && button == HIGH) {
    button_state = BUTTON_NUM_STATE * 2;
    return HIGH;
  }
  if (button_state > BUTTON_NUM_STATE && button_state <= BUTTON_NUM_STATE *2 && button == LOW) {
    button_state = button_state - 1;
    return HIGH;
  }
  if (button_state > BUTTON_NUM_STATE && button_state < BUTTON_NUM_STATE *2 && button == HIGH) {
    button_state = button_state + 1;
    return HIGH;
  }
  if (button_state == BUTTON_NUM_STATE * 2 && button == HIGH) {
    return HIGH;
  }  
}

void ButtonState::buttonReset() {
  button_state = 0;
  digitalWrite(pin, LOW);
}

