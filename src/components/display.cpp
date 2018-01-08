#include "display.h"
#include <Wire.h>
#include <SPI.h>
#include <pins_arduino.h>

#define FONT_HEIGHT   11
#define FONT_WIDTH     6

void Display::begin() {
  Wire.begin();
  display.reset(new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0));
  display->begin();
  display->setFont(u8g2_font_profont11_mf);	// choose a suitable font
  display->clearBuffer();
  display->sendBuffer();
}

void Display::clear() {
  display->clearDisplay();
  display->clearBuffer();
}

void Display::printXY(int x, int y, String msg) {
  display->drawStr(x*FONT_WIDTH, (y+1)*FONT_HEIGHT, msg.c_str());	// write something to the internal memory
  display->sendBuffer();
}

void Display::printXY(int x, int y, const char* msg) {
  display->drawStr(x*FONT_WIDTH, (y+1)*FONT_HEIGHT, msg);	// write something to the internal memory
  display->sendBuffer();
}


void Display::dotXY(int x, int y) {
  display->drawPixel(x, y);	// write something to the internal memory
  display->sendBuffer();
}
