#ifndef LEDButton_h
#define LEDButton_h

#include <Arduino.h>
#include "Button.h"

class LEDButton : public Button
{
  private:

    const uint8_t _ledOutPin_;

  public:

    // Call Button(int, int) constructor using constructor initialization list, setting inherited member data for you:
    LEDButton(const uint8_t pin,
              const uint8_t ledOutPin,
              unsigned int debounceDelayDuration=500);
    
    void begin();

    const void turnLEDOn();
    const void turnLEDOff();
    const void toggleLED();
    
};

#endif