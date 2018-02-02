#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <Arduino.h>
#include <OneButton.h>
#include <Wire.h>
#include "components/display.h"
#include "server/server.h"
#include "utils/utils.h"

int state;
std::unique_ptr<OneButton>      button;
std::unique_ptr<ServerIndoor>   server;
std::shared_ptr<Display>        display;

void singleClick();
void longClick();
void serverCallback(ServerIndoor *server, ServerState serverState);

void setup() {
  state = 0;
  Serial.begin(9600);
  while (!Serial);
  button.reset(new OneButton(D4, true));
  button->attachClick(singleClick);
  button->attachLongPressStart(longClick);

  display = std::make_shared<Display>();
  display->begin();

  server.reset(new ServerIndoor());
  server->setConnectTimeout(20000);
  server->setCallback(&serverCallback);
  server->start();
}

void showClock() {
  if (state == 2) {
    ClockValue clock = server->hw()->readClock();
    display->printXY(0, 2, "Fecha: " + getDateDMY(clock));
    display->printXY(0, 3, " Hora:  " + getTime(clock));
  }
}

void showInfo() {
  if (state == 1) {
    display->clear();
    display->printXY(4, 0, ">> Indoor <<");
    display->printXY(0, 2, "SSID: " + server->getSSID());
    display->printXY(0, 3, "IP: " + server->getIP());
    return;
  }
  if (state == 2) {
    display->clear();
    display->printXY(4, 0, ">> Indoor <<");
    showClock();
    return;
  }
  if (state == 3) {
    display->clear();
    display->printXY(4, 0, ">> Indoor <<");
    for (int i = 0; i < server->hw()->getSensorNumber(); i++) {
      String name = server->hw()->getSensorName(i);
      String buf = server->hw()->printSensor(name);
      display->printXY(0, i+1, buf);
    }
    return;
  }
  if (state == 4) {
    display->clear();
    display->printXY(4, 0, ">> Indoor <<");
    for (int i = 0; i < server->hw()->getSwitchNumber(); i++) {
      String name = server->hw()->getSwitchName(i);
      display->printXY(0, i+1, server->hw()->printSwitch(name));
    }
    return;
  }
}

void singleClick() {
  state++;
  if (state == 5) {
    state = 1;
  }
  showInfo();
}

void longClick() {
}

void loop() {
  button->tick();
  server->loop();
  showClock();
  delay(100);
}

void serverCallback(ServerIndoor *server, ServerState serverState) {
  if (serverState.state == DISCONNECTED) {
    display->clear();
    display->printXY(4, 0, ">> Indoor <<");
    display->printXY(0, 1, "Desconectado.       ");
    state = 0;
    return;
  }
  if (serverState.state == WAITING_CONNECT) {
    display->clear();
    display->printXY(4, 0, ">> Indoor <<");
    display->printXY(0, 1, "Conectando...        ");
    state = 0;
    return;
  }
  if (serverState.state == CONNECTED) {
    state = 1;
    showInfo();
    return;
  }
}
