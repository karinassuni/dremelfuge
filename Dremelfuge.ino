/*
Dremelfuge ETC Project 2015-2016
Adapted from Orlov's Instructable on Arduino Powered Centrifuge

Compiled and Edited by: Albert Ju, Fu Yang Chin
*/

#include <LiquidCrystal.h>               // include the library code for display
                                         // because it's a file inside of the Arduino.app 's library folder, use <> DIRECTIVE
#include "LEDButton.h"                   // include custom library local to the .ino--within the same sketch folder
                                         // because it's a relative path in the same directory, use "" LITERAL PATHNAME

//---GLOBAL VARIABLES--//
//:: ***these variables MUST be global because they're being used by multiple totally separate functions--static local won't do
LiquidCrystal lcd = LiquidCrystal(12, 11, 5, 4, 3, 6);   // initialize the library with the numbers of the interface pins

// White Pushbutton
const uint8_t WPB_IN_PIN = 2, WPB_LED_PIN = 13;
const unsigned int WPB_DEBOUNCE_DELAY = 500;
LEDButton wpb = LEDButton(WPB_IN_PIN, WPB_LED_PIN, WPB_DEBOUNCE_DELAY);

// PWM (Pulse-Width Modulation) Motor Control and Potentiometer
const uint8_t MOTOR_PIN = 9;
const uint8_t POT_PIN = A5;


//*** No magic numbers! Using case statements, enum will provide readable, self-documenting code for when switching modes
enum class Mode
{
  SETTING_TIME,
  SETTING_SPEED,
  SPINNING
};
Mode mode = Mode::SETTING_TIME;                       //*** initialization over assignment; set SETTING_TIME to be the first mode!

//---Function prototypes---//
inline void lcdPrintFormattedSecs(unsigned long seconds);

void setup()
{
  const uint8_t LCD_COLUMNS = 20, LCD_ROWS = 4;

  wpb.begin();
  wpb.turnLEDOn();

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);

  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  lcd.setCursor(0, 0);
  lcd.print(F("  Dremel Centrifuge"));                //*** Serial or derived functions will store their String arguments in RAM, which is wasteful; F() stores them in flash memory, where the sketch is stored (largest storage)
  lcd.setCursor(0, 2);
  lcd.print(F("Set speed:"));
  lcd.setCursor(0, 3);
  lcd.print(F("   Push to Start!"));

  Serial.begin(9600);
}

void loop()
{
                                                      //*** static local variables--i.e. static variables inside of functions--are NOT STACK VARIABLES;
                                                      //    their values persist even after function ends, as long as the program is still running
                                                      //*** also, static local variables still only have local scope
                                                      //*** perform operations with data of the same type, so as to boost performance by not needing to typecast
  static unsigned long countdown;                     // the duration the centrifuge should run
  static unsigned long spinningStartTime;             // point on the timeline where the countdown starts, i.e. when wpb is pressed the second time
  static uint8_t motorSpeed;

  switch(mode)
  {
    case Mode::SETTING_TIME:
    {
      countdown = map(analogRead(POT_PIN), 0, 1024, 0, 901);

      lcd.setCursor(0, 1);
      lcd.print(F("Set time: <"));
      lcdPrintFormattedSecs(countdown);
      lcd.print(F(">   "));

      if(wpb.pressed())
      {
        // then implicitly set in stone the countdown value, change display, and enter next block
        //*** because mode will change, countdown will no longer be updated by pot, so on case switch countdown will equal its final value, no further updating necessary

        // Erase <> selector braces
        lcd.setCursor(10, 1);
        lcdPrintFormattedSecs(countdown);
        lcd.print(F("    "));

        countdown*= (long)1000;                       //*** to save space, the ATmega328P chip of UNO uses 2-byte ints, which can hold at max 32 767; operator*(int, int) will still return an int, but in this case an overflowed int;
                                                      //*** initSetTime, a long, will just be assigned the overflowed value--it won't force operator*(int, int) to return a long.
                                                      //*** so, by typecasting 1000 or using 1000L, you make the compiler promote analogRead() to a long, so operator*(long, long) will return the expected value, within range of sizeof long
        mode = Mode::SETTING_SPEED;
      }
      break;
    }


    case Mode::SETTING_SPEED:
    {
      motorSpeed = map(analogRead(POT_PIN), 0, 1024, 0, 255);

      lcd.setCursor(11, 2);
      lcd.print(F("<"));
      //map printing of motorSpeed to rpm range?!
      lcd.print(motorSpeed);
      lcd.print(F(">  "));

      if(wpb.pressed())
      {
        // then implicitly set in stone the motorSpeed value, change display, pre-start the countdown, and switch block

        // Erase <> selector braces
        lcd.setCursor(11, 2);
        lcd.print(motorSpeed);
        lcd.print(F("  "));

        spinningStartTime = millis();

        mode = Mode::SPINNING;
      }
      break;
    }


    case Mode::SPINNING:
    {
      unsigned long secondsLeft = (countdown - (millis() - spinningStartTime))/1000;  //*** save RAM AND flash memory by only doing this calculation once per case in loop()
      analogWrite(MOTOR_PIN, motorSpeed);

      lcd.setCursor(0, 1);
      lcd.print(F("Finished in: "));
      lcdPrintFormattedSecs(secondsLeft);
      lcd.print(F("  "));

      if(wpb.pressed() || secondsLeft == 0)
      {
        digitalWrite(MOTOR_PIN, LOW);
        mode = Mode::SETTING_TIME;
      }
      break;
    }
  }
}

inline void lcdPrintFormattedSecs(unsigned long seconds) //*** inline = suggestion to the compiler to paste the raw code of this function wherever it's called == performance optimization, no hopping pointers to callers and callbacks
{
  //***
  lcd.print(seconds/60); lcd.print(F(":")); lcd.print((seconds%60 < 10 ? F("0") : F(""))); lcd.print(seconds%60);
}
