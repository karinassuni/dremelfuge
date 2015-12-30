// prevent Button class from being linked twice by the compiler, in case multiple child classes of Button are included in a sketch:
#ifndef Button_h
// define an empty token Button_h to keep track of whether the Button class was already linked:
#define Button_h

#include "Arduino.h"

class Button
{
  protected:
    const uint8_t pin;
    uint16_t debounceDelay;

    bool defaultState;
    unsigned long lastTimePressed;
  public:
    //NOTE: initialization list only included in .cpp
    Button(const uint8_t pin, uint16_t debounceDelay=500);
    virtual void begin();
    void setDebounceDelay(int debounceDelay);

    bool pressed();
    const void toggleWhenPressed(bool& condition);      // `const` member fn means that this fn *will not modify `this` member data*
    const bool isHeld();
};

#endif