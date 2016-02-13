#ifndef LEDButton_h
#define LEDButton_h

#include <Arduino.h>
#include "Button.h"

class LEDButton : public Button
{
  private:

    const uint8_t ledPin;

  public:

    LEDButton(uint8_t inPin, uint8_t ledPin, unsigned int debounce=500);
    
    void begin();

    const void ledOn();
    const void ledOff();
    const void ledToggle();
    
};

#endif