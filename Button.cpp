#include "Arduino.h"
#include "Button.h"

// Combination of Button(byte) and Button(byte, int) constructors via default args
Button::Button(const uint8_t pin, uint16_t debounceDelay)
: _pin(pin)
, _debounceDelay(debounceDelay)
, _lastTimePressed(0)
{} // Constructor

/* Separated from constructor because these functions need to be called in
  setup(), and object needs global scope */

void Button::begin()
{
  pinMode(_pin, INPUT);
  _defaultState = digitalRead(_pin);
}

void Button::setDebounceDelay(uint16_t debounceDelay)
{
  _debounceDelay = debounceDelay;
}


bool Button::pressed()
{
  bool currentState = digitalRead(_pin);

  if(currentState == !_defaultState && (millis() - _lastTimePressed > _debounceDelay))
  {
    _lastTimePressed = millis();
    return true;
  }
  else
    return false;
} 

const void Button::toggleWhenPressed(bool& condition)
{
  if(pressed())
    condition ^= true;                                  // == condition = !condition; using a more efficient "xor toggle"
} 

const bool Button::isHeld()
{
  return digitalRead(_pin) == !_defaultState;
} 
