// Still accessing Arduino functions and constants, still need to #include--Button.h's #include Arduino.h DOES NOT APPLY:
#include <Arduino.h>
#include "LEDButton.h"

// The exact same constructor as a regular Button, but with an LED pin:
LEDButton::LEDButton(const uint8_t pin,
                     const uint8_t ledOutPin,
                     unsigned int debounceDelayDuration)
: Button(pin, debounceDelayDuration)
, _ledOutPin_(ledOutPin)
{} // Constructor

void LEDButton::begin()
{
  Button::begin();                                      // call base class version of begin()
  pinMode(_ledOutPin_, OUTPUT);
  digitalWrite(_ledOutPin_, LOW);                        // LED off by default
}

const void LEDButton::turnLEDOn()
{
  digitalWrite(_ledOutPin_, HIGH);
}

const void LEDButton::turnLEDOff()
{
  digitalWrite(_ledOutPin_, LOW);
}

const void LEDButton::toggleLED()
{
  digitalWrite(_ledOutPin_, digitalRead(_ledOutPin_) ^ 1);                       // bool ^ 1 == !bool, except faster; a "xor toggle"
}