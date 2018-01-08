#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <Arduino.h>
#include <OneButton.h>
#include <Wire.h>
#include "components/display.h"
#include "server/server.h"
#include "serverAP/serverAP.h"

#define SERVER    0
#define SERVER_AP 1

int state;
std::unique_ptr<OneButton>      button;
std::unique_ptr<ServerIndoor>   server;
std::unique_ptr<ServerAP>       serverAP;
std::shared_ptr<Display>        display;

void singleClick();
void longClick();

void setup() {
  Serial.begin(9600);
  while (!Serial);
  state = SERVER;
  button.reset(new OneButton(D4, true));
  button->attachClick(singleClick);
  button->attachLongPressStart(longClick);
  display = std::make_shared<Display>();
  display->begin();
  server.reset(new ServerIndoor(display));
  server->start();
}

void singleClick() {
  if (state == SERVER) {
    server->click();
  }
}

void longClick() {
  if (state == SERVER) {
    server->stop();
    server.reset();

    state = SERVER_AP;
    display->clear();
    serverAP.reset(new ServerAP(display));
    serverAP->start();
  } else
  if (state == SERVER_AP) {
    serverAP->stop();
    serverAP.reset();

    state = SERVER;
    display->clear();
    server.reset(new ServerIndoor(display));
    server->start();
  }
}

void loop() {
  button->tick();
  if (state == SERVER) {
    server->loop();
  } else
  if (state == SERVER_AP) {
    serverAP->loop();
  }
  delay(100);
}
