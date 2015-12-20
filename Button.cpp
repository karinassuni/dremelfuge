#include "Arduino.h"
#include "Button.h"

//:: combination of Button(byte) and Button(byte, bool) and Button(byte, bool, int) constructors via default args
Button::Button(const uint8_t pin, const bool isToggleButton, uint16_t debounceDelay)
: pin(pin), isToggleButton(isToggleButton)
{
  this->debounceDelay = debounceDelay;
  lastTimePressed = 0;
}

//:: ***separated from constructor bc these functions need to be called in setup(), and object needs global scope
void Button::begin()
{
  pinMode(pin, INPUT);
  defaultState = digitalRead(pin);
  currentState = digitalRead(pin);
}

void Button::setDebounceDelay(int debounceDelay)
{
  this->debounceDelay = debounceDelay;
}


bool Button::pressed()
{
  currentState = digitalRead(pin);
  if(currentState == !defaultState && (millis() - lastTimePressed > debounceDelay || isToggleButton == true))
  {
    lastTimePressed = millis();
    return true;
  }
  else
    return false;
}
void Button::toggleWhenPressed(bool& condition)
{
  if(isToggleButton)
    condition = digitalRead(pin);
  else if(pressed())
    condition = !condition;
}
//:: same behavior as pressed() for toggle buttons
bool Button::held()
{
  return digitalRead(pin) == !defaultState;
}
