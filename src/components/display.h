#ifndef DISPLAY_H
#define DISPLAY_H
#include <memory>
#include <U8g2lib.h>

class Display {
private:
  std::unique_ptr<U8G2_SH1106_128X64_NONAME_F_HW_I2C> display;

public:
  void begin();

  void clear();

  void printXY(int x, int y, String msg);

  void printXY(int x, int y, const char* msg);

  void dotXY(int x, int y);
};

#endif
