#include "Arduino.h"               // Still accessing Arduino functions and constants, still need to
                                   // #include--Button.h's #include Arduino.h DOES NOT APPLY
#include "LEDButton.h"

//:: The exact same constructor as a regular Button, but with an LED pin
LEDButton::LEDButton(const uint8_t pin, const uint8_t ledOutPin, uint16_t debounceDelay)
: Button(pin, debounceDelay), ledOutPin(ledOutPin) {}
void LEDButton::begin()
{
  Button::begin();                 // call base class version of begin()
  pinMode(ledOutPin, OUTPUT);
  digitalWrite(ledOutPin, LOW);    // LED off by default
}


void LEDButton::turnLEDOn()
{
  digitalWrite(ledOutPin, HIGH);
}
void LEDButton::turnLEDOff()
{
  digitalWrite(ledOutPin, LOW);
}
void LEDButton::toggleLED()
{
  digitalWrite(ledOutPin, digitalRead(ledOutPin) ^ 1);   // == digitalRead(ledOutPin, !digitalRead(ledOutPin),
                                                         // except faster; a "xor toggle"
}