#include <Arduino.h>
#include "Button.h"

Button::Button(uint8_t inPin, unsigned int debounce)
: inputPin(inPin)
, debounceDelay(debounce)
, lastPressTime(0)
{} 


void Button::begin() {

    // Separated from constructor as per Arduino's API style guide

    pinMode(inputPin, INPUT);
    defaultState = digitalRead(inputPin);

}

void Button::setDebounce(unsigned int debounce) {

    debounceDelay = debounce;

}

bool Button::pressed() {

    if(digitalRead(inputPin) == !defaultState
         && millis() - lastPressTime > debounceDelay) // delta t since last press > debounceDelay
    {
        lastPressTime = millis();
        return true;
    }
    
    return false;

} 

const bool Button::held() {
    
    // pressed() without debounce

    return (digitalRead(inputPin) == !defaultState);
} 
