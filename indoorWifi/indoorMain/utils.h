#ifndef H_UTILS
#define H_UTILS

class LedBlink {
  public:
    LedBlink(int p);  
    void blink(int interval);
    void on();
    void off();
    
  private:
    unsigned long previousMillis;
    int ledState;
    int pin;
};

class ButtonState {

  public:
    ButtonState(int p);

    int buttonPressed();
    void buttonReset();
    
  private:
    int button_state = 0;
    int pin;
};

#endif
