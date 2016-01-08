// Still accessing Arduino functions and constants, still need to #include--Button.h's #include Arduino.h DOES NOT APPLY:
#include "Arduino.h"
#include "LEDButton.h"

// The exact same constructor as a regular Button, but with an LED pin:
LEDButton::LEDButton(const uint8_t pin,
                     const uint8_t ledOutPin,
                     uint16_t debounceDelay)
: Button(pin, debounceDelay)
, _ledOutPin(ledOutPin)
{} // Constructor

void LEDButton::begin()
{
  Button::begin();                                      // call base class version of begin()
  pinMode(_ledOutPin, OUTPUT);
  digitalWrite(_ledOutPin, LOW);                        // LED off by default
}


const void LEDButton::turnLEDOn()
{
  digitalWrite(_ledOutPin, HIGH);
}

const void LEDButton::turnLEDOff()
{
  digitalWrite(_ledOutPin, LOW);
}

const void LEDButton::toggleLED()
{
  digitalWrite(_ledOutPin, digitalRead(_ledOutPin) ^ 1);// bool ^ 1 == !bool, except faster; a "xor toggle"
}