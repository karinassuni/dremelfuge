#include "Arduino.h"
#include "Button.h"

//:: combination of Button(byte) and Button(byte, int) constructors via default args
Button::Button(const uint8_t pin, uint16_t debounceDelay)
: pin(pin)
{
  this->debounceDelay = debounceDelay;
  lastTimePressed = 0;
}

//:: ***separated from constructor bc these functions need to be called in setup(), and object needs global scope
void Button::begin()
{
  pinMode(pin, INPUT);
  defaultState = digitalRead(pin);
}

void Button::setDebounceDelay(int debounceDelay)
{
  this->debounceDelay = debounceDelay;
}


bool Button::pressed()
{
  bool currentState = digitalRead(pin);

  if(currentState == !defaultState && (millis() - lastTimePressed > debounceDelay))
  {
    lastTimePressed = millis();
    return true;
  }
  else
    return false;
}
void Button::toggleWhenPressed(bool& condition)
{
  if(pressed())
    condition ^= true;   // == condition = !condition ; using a more efficient "xor toggle"
}
bool Button::isHeld()
{
  return digitalRead(pin) == !defaultState;
}
