#ifndef Button_h        // prevent Button class from being linked twice by the compiler, in cases where multiple child classes of Button are included in a sketch
#define Button_h        // define an empty token Button_h to keep track of whether the Button class was already linked
#include "Arduino.h" 

class Button
{
  protected:
    const uint8_t pin;
    const bool isToggleButton;
    uint16_t debounceDelay;

    bool defaultState;
    bool currentState;
    unsigned long lastTimePressed;
  public:
    //NOTE: initialization list only included in .cpp
    Button(const uint8_t pin, const bool isToggleButton=false, uint16_t debounceDelay=500);
    virtual void begin();
    void setDebounceDelay(int debounceDelay);

    bool pressed();
    void toggleWhenPressed(bool& condition);
    bool held();    
};

#endif