// prevent Button class from being linked twice by the compiler, in case multiple child classes of Button are included in a sketch:
#ifndef Button_h
// define an empty token Button_h to keep track of whether the Button class was already linked:
#define Button_h

#include <Arduino.h>

class Button
{
  protected:
    
    const uint8_t _pin_;
    // Length of time to wait before checking button state again
    unsigned int _debounceDelayDuration_;

    bool _defaultState_;
    unsigned long _lastTimePressed_;

  public:

    //NOTE: initialization list only included in .cpp
    Button(const uint8_t pin,
           unsigned int debounceDelayDuration=500);

    virtual void begin();
    void setDebounceDelay(uint16_t debounceDelayDuration);

    bool pressed();
    const bool isHeld();                                // `const` member fn means that this fn *will not modify `this` member data*

};

#endif