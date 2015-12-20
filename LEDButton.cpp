#include "Arduino.h" // Still accessing Arduino functions and constants, still need to #include--Button.h's #include Arduino.h DOES NOT APPLY
#include "LEDButton.h"

LEDButton::LEDButton(const uint8_t pin, const uint8_t ledOutPin, uint16_t debounceDelay) : Button(pin, false, debounceDelay), ledOutPin(ledOutPin)
{
  //LED off by default
  ledState = LOW;
}
void LEDButton::begin()
{
  Button::begin();                      // call base class version of begin()
  pinMode(ledOutPin, OUTPUT);
  digitalWrite(ledOutPin, ledState);
}


void LEDButton::turnLEDOn()
{
  ledState = HIGH;
  digitalWrite(ledOutPin, ledState);
}
void LEDButton::turnLEDOff()
{
  ledState = LOW;
  digitalWrite(ledOutPin, ledState);
}
void LEDButton::toggleLED()
{
  ledState = !ledState;
  digitalWrite(ledOutPin, ledState);
}