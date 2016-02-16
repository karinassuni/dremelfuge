/*
Dremelfuge ETC Project 2015-2016
Adapted from Orlov's Instructable on Arduino Powered Centrifuge

Compiled and Edited by: Albert Ju, Fu Yang Chin, Karina Antonio

Uploaded to an Arduino UNO, with MCU ATmega328P-PU.
Arduino UNO specs: https://www.arduino.cc/en/Main/ArduinoBoardUno
ATmega328P-PU datasheet: http://www.atmel.com/Images/doc8161.pdf

Sketch uses 4,398 bytes (13%) of program storage space. Maximum is 32,256 bytes.
Global variables use 88 bytes (4%) of dynamic memory. Maximum is 2,048 bytes.
*/

#include <LiquidCrystal.h>  // library inside of Arduino.app, use <> DIRECTIVE
#include "LEDButton.h"      // library not in Arduino.app but in sketch folder, use "" LITERAL PATHNAME
#include "LCDPrinter.h"     // Extension of LiquidCrystal, with special higher-level print functions
#include "Printers.cpp"     // Functors for customized print formatting for classes that extend Arduino's `Print` class
#include <avr/pgmspace.h>   // macros for storing data in Flash instead of RAM, and fetching it from Flash

namespace {

    const uint8_t MOTOR_PIN = 10;   // connected to base of transistor
    const uint8_t POT_PIN = A5;

    // White PushButton
    const uint8_t WPB_INPUT_PIN = 2;
    const uint8_t WPB_LED_PIN = 13;
    const unsigned int WPB_DEBOUNCE_DELAY = 500;
    LEDButton wpb = LEDButton(WPB_INPUT_PIN, WPB_LED_PIN, WPB_DEBOUNCE_DELAY);

    // LCD
    LCDPrinter lcd = LCDPrinter(12, 11, 5, 4, 3, 6);    // numbers of the interface pins
    const uint8_t LCD_COLUMNS = 20;
    const uint8_t LCD_ROWS = 4;

    PercentPrint percent;
    FSecsPrint fsecs;  

    namespace UI {

        /* Explanation of need for `constexpr`:
            We need to place the time, speed, and instruction strings at certain
            indices after the UI prompt. Instead of hardcoding these values and instead
            of wasting global memory on these indices, `constexpr`s are used--the
            indices will be calculated at compile time and infused into the instruction
            set, instead of taking up memory.
        */

        // Default, set-up UI
            const char title[] PROGMEM      = "  Dremel Centrifuge";
        constexpr char setTime[] PROGMEM    = "Set time: ";
        constexpr char setSpeed[] PROGMEM   = "Set speed: ";
           constexpr char pushTo[] PROGMEM  = "   Push to ";

        const char pushStart[] PROGMEM = "Start!";
        const char nullValue[] PROGMEM = "---";

        // SPINNING mode UI
        constexpr char finishedIn[] PROGMEM = "Finished in: ";
        const char pushStop[] PROGMEM = "Stop! ";

        // Value decoration--very small, storing in Flash would be less efficient
        // due to the extra time and extra code needed to access Flash memory
        const char selected[]   = "<>";

        constexpr uint8_t length(const char* string)
        {
            return *string ? 1 + length(string + 1) : 0;
        }

        // Indices at which to print user-set values
        constexpr uint8_t setTimeIndex = length(setTime);
        constexpr uint8_t speedIndex = length(setSpeed);
        constexpr uint8_t finishTimeIndex = length(finishedIn); 
        constexpr uint8_t instructionIndex = length(pushTo);   

    } // namespace UI

    // LCD row indices
    enum line : char
    {
        Title = 0,
        Time,
        Speed,
        Instructions
    };

} // namespace

inline void changeUI(PGM_P timeStr, PGM_P instructionStr) {

    // "Set time:" <=> "Finished in:"
    lcd.changeLine_P(timeStr, line::Time);

    // "Push to Start!" <=> "Push to Stop!"
    lcd.setCursor( UI::instructionIndex, line::Instructions);
    lcd.print_P(instructionStr);

}

template <typename T, class Functor>
inline void selectPrint(const T value, Functor& printfn,
                        uint8_t col, uint8_t row) {

    lcd.setCursor(col, row);

    lcd.print( UI::selected[0] );
    printfn(value, lcd);
    lcd.print( UI::selected[1] );

    // Clear hanging digits
    lcd.print(F("   "));

}

template <typename T, class Functor>
inline void deselectPrint(const T value, Functor& printfn,
                          uint8_t col, uint8_t row) {

    lcd.setCursor(col, row);

    printfn(value, lcd);

    // Clear hanging digits
    lcd.print(F("   "));

}

/////////////////////////////////////////////////////////////////////////////////

void setup() {

    lcd.begin(LCD_COLUMNS, LCD_ROWS);

    wpb.begin();
    wpb.ledOn();

    pinMode(MOTOR_PIN, OUTPUT);
    pinMode(POT_PIN, INPUT);

    // Motor is OFF on startup
    digitalWrite(MOTOR_PIN, LOW);

    // Print starter UI
    using namespace UI;
    lcd.changeLine_P(title, line::Title);
    lcd.changeLine_P(setTime, line::Time);
    lcd.changeLine_P(setSpeed, line::Speed); lcd.print_P(nullValue);
    lcd.changeLine_P(pushStart, line::Instructions); 

}

/////////////////////////////////////////////////////////////////////////////////

void loop() {

    enum class Mode : char
    {
        SETTING_TIME,
        SETTING_SPEED,
        SPINNING
    };

    static Mode currentMode = Mode::SETTING_TIME;
    static unsigned long setDuration;   // in milliseconds
    static unsigned long spinningStartTime;
    static uint8_t motorSpeed;  // [0, 255] for analogWrite

    switch(currentMode) {

        case Mode::SETTING_TIME: {

            setDuration = map(analogRead(POT_PIN), 0, 1024, 0, 900);    // Max time is 15 minutes

            selectPrint(setDuration, fsecs, UI::setTimeIndex, line::Time);

            if(wpb.pressed()) {

                setDuration *= 1000;    // convert for calculations (printing done)
                deselectPrint(setDuration, fsecs, UI::setTimeIndex, line::Time);                      
                currentMode = Mode::SETTING_SPEED;

            } 

            break;

        }

        case Mode::SETTING_SPEED: {

            motorSpeed = map(analogRead(POT_PIN), 0, 1024, 0, 255);
            const uint8_t motorSpeedPercent = map(motorSpeed, 0, 255, 0, 100);  // for printing

            selectPrint(motorSpeedPercent, percent, UI::speedIndex, line::Speed);

            if(wpb.pressed()) {

                deselectPrint(motorSpeedPercent, percent, UI::speedIndex, line::Speed);
                changeUI(UI::finishedIn, UI::pushStop);
                currentMode = Mode::SPINNING;
                spinningStartTime = millis();

            }

            break;

        }

        case Mode::SPINNING: {

            // analogWrite/PWM must be done continuously
            analogWrite(MOTOR_PIN, motorSpeed);

            // Store in a variable so calculation is only done once
            const unsigned long timeLeft = (setDuration - (millis() - spinningStartTime))/1000;

            selectPrint(timeLeft, fsecs, UI::finishTimeIndex, line::Time);

            if(wpb.pressed() || timeLeft <= 0) {

                digitalWrite(MOTOR_PIN, LOW);
                changeUI(UI::setTime, UI::pushStart);
                currentMode = Mode::SETTING_TIME;

            }

            break;

        }

    } // switch(mode)

} // void loop()