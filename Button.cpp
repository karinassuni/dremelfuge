#include "Arduino.h"
#include "Button.h"

// Combination of Button(byte) and Button(byte, int) constructors via default args
Button::Button(const uint8_t pin, uint16_t debounceDelay)
: pin(pin)
, debounceDelay(debounceDelay)
{
  lastTimePressed = 0;
} // Constructor

/* Separated from constructor because these functions need to be called in
  setup(), and object needs global scope */
void Button::begin()
{
  pinMode(pin, INPUT);
  defaultState = digitalRead(pin);
} // begin()

void Button::setDebounceDelay(uint16_t debounceDelay)
{
  this->debounceDelay = debounceDelay;
} // setDebounceDelay(int)


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
} // pressed()

const void Button::toggleWhenPressed(bool& condition)
{
  if(pressed())
    condition ^= true;                                  // == condition = !condition; using a more efficient "xor toggle"
} // toggleWhenPressed()

const bool Button::isHeld()
{
  return digitalRead(pin) == !defaultState;
} // isHeld()
