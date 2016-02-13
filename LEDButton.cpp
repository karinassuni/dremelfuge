#include <Arduino.h>
#include "LEDButton.h"

LEDButton::LEDButton(uint8_t inPin, uint8_t ledPin, unsigned int debounce)
: Button(inPin, debounce)
, ledPin(ledPin)
{}

void LEDButton::begin() {

    Button::begin();
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);    // LED off by default

}

const void LEDButton::ledOn() {

    digitalWrite(ledPin, HIGH);

}

const void LEDButton::ledOff() {

    digitalWrite(ledPin, LOW);

}

const void LEDButton::ledToggle() {

    digitalWrite(ledPin, !digitalRead(ledPin));

}