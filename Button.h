#ifndef Button_h
#define Button_h

#include <Arduino.h>

class Button
{
  protected:
    
    const uint8_t inputPin;
    unsigned int debounceDelay;
    bool defaultState;
    unsigned long lastPressTime;

  public:

    Button(uint8_t inPin, unsigned int debounce=500);

    virtual void begin();
    void setDebounce(uint16_t debounce);

    bool pressed();
    const bool held();

};

#endif