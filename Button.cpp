#include <Arduino.h>
#include "Button.h"

// Combination of Button(byte) and Button(byte, int) constructors via default args
Button::Button(const uint8_t pin, unsigned int debounceDelayDuration)
: _pin_(pin)
, _debounceDelayDuration_(debounceDelayDuration)
, _lastTimePressed_(0)
{} // Constructor

/* Separated from constructor because these functions need to be called in
  setup(), and object needs global scope */

void Button::begin()
{
  pinMode(_pin_, INPUT);
  _defaultState_ = digitalRead(_pin_);
}

void Button::setDebounceDelay(unsigned int debounceDelayDuration)
{
  _debounceDelayDuration_ = debounceDelayDuration;
}

bool Button::pressed()
{
  bool currentState = digitalRead(_pin_);
  unsigned long durationSinceLastPress = millis() - _lastTimePressed_;

  if(currentState == !_defaultState_ && durationSinceLastPress > _debounceDelayDuration_)
  {
    _lastTimePressed_ = millis();
    return true;
  }
  else
    return false;
} 

// Same as pressed(), but without any debounce delay--hence good for held buttons
const bool Button::isHeld()
{
  // return true if button held, false if not (if currently at default state)
  return digitalRead(_pin_) == !_defaultState_;
} 
