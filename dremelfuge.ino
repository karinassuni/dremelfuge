/*
Dremelfuge ETC Project 2015-2016
Adapted from Orlov's Instructable on Arduino Powered Centrifuge

Compiled and Edited by: Albert Ju, Fu Yang Chin

Uploaded to an Arduino UNO, with MCU ATmega328P-PU.
Arduino UNO specs: https://www.arduino.cc/en/Main/ArduinoBoardUno
ATmega328P-PU datasheet: http://www.atmel.com/Images/doc8161.pdf
*/

#include <LiquidCrystal.h>                              // library inside of Arduino.app, use <> DIRECTIVE
#include "LEDButton.h"                                  // library not in Arduino.app but in sketch folder, use "" LITERAL PATHNAME
#include "karina_utility.cpp"                           // for void printFSecs(unsigned long seconds, Stream& stream)

//============================GLOBAL VARIABLES=================================//
/* *** These variables MUST be global because they're being used by multiple
  totally separate functions--static local won't do
  i.e., this data NEEDS to be available to ALL of the program at ALL times
  */
/* *** The following anonymous namespace ensures internal linkage of all global
  variables and functions, as if all globals were declared `static` (see large
  comment in loop() for details on `static`)
  */

namespace
{
  LiquidCrystal lcd = LiquidCrystal(12, 11, 5, 4, 3, 6);                         // numbers of the interface pins

  // White PushButton
  const uint8_t WPB_IN_PIN = 2, WPB_LED_PIN = 13;
  const unsigned int WPB_DEBOUNCE_DELAY = 500;
  LEDButton wpb = LEDButton(WPB_IN_PIN, WPB_LED_PIN, WPB_DEBOUNCE_DELAY);        // cannot be `const`, member data will change

  const uint8_t MOTOR_PIN = 9;
  const uint8_t POT_PIN = A5;

  //Function prototypes

  /* It's good practice to define the prototypes yourself, rather than letting
    the compiler auto-generate them for you
    */

} // namespace

// For some reason, only works with extension .h--related to order of compilation
#include "printers.h"                                   // relies on the existence of a variable `LiquidCrystal lcd`


/////////////////////////////////////////////////////////////////////////////////

void setup()
{
  const uint8_t LCD_COLUMNS = 20, LCD_ROWS = 4;
  lcd.begin(LCD_COLUMNS, LCD_ROWS);

  wpb.begin();
  wpb.turnLEDOn();

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);

  /* Explanation of `F()` macro and contants/literals:
    Note: Literals are not temporaries.
    All literals, at least in C, are obviously constants--their values can't be
    changed during program execution--and so in the programming sense are
    unnamed, unchangeable "variables." And like all variables, literals are
    stored in memory (RAM by default). However, in order for the value of a
    literal to always be remembered each time the value is needed in the current
    scope (in a recurring function call for instance), literals are also
    statically allocated (see giant comment below for more info on `static`). So,
    `int bar = 17;` is the same as `static const int foo = 17; bar = foo;`--
    Using unnamed literals is the exact same thing as using const variables,
    except the former is worse because literal values are unnamed and so without
    context, making them seem "magical" where they appear.

    Literals being stored in RAM is problematic when dealing with string
    literals, which are const char arrays that can take up a lot of RAM without
    contributing anything to the program! All we want to do with string literals
    is to print them, but they come at a huge RAM cost, so does that mean that we
     shouldn't make any MCU projects with text or GUI involved? No! Enter `F()`:
    `F()` is a macro (for more complex inline code) for AVR MCUs that tells the
    compiler to store string literals in flash memory instead of RAM, and when it
    comes time to use that string literal in your program, the MCU will retrieve
    the string literals from Flash instead of RAM as well. Since variables are
    stored in RAM by default, memory addresses in source code are also read from
    RAM by default, so `F()` has to address both behaviors.
    `F()` saves you a ton of RAM by storing and reading string literals (and only
    string literals) in/from flash memory instead of in/from RAM!
    */

  // Initialize LCD UI
  lcd.setCursor(0, 0);
  lcd.print(F("  Dremel Centrifuge"));
  lcd.setCursor(0, 2);
  lcd.print(F("Set speed: ---"));
  lcd.setCursor(0, 3);
  lcd.print(F("   Push to "));

  // Timer interrupt implementation + explanation removed; see commit f587[...]

} // void setup()

/////////////////////////////////////////////////////////////////////////////////

void loop()
{

  /* Explanation of `static` and statically allocated memory in C++:

    3 types of things can be keyword `static`: (local) function variables & class
    member data (variables and functions--functions = pointer to address of
    instructions). But `static` gets implemented the same way in each. Global
    variables are similar to keyword `static` data in that both are "statically
    allocated." This means two things: 1. globals and `static`s are *initialized
    at the start of the program, before main*, and 2. global and `static` data
    are *given PERMANENT memory addresses in RAM outside of the stack and heap,*
    so that a) even if a local `static` variable goes out of scope, its memory
    will not be erased because it is located separately from its collapsed
    stack frame; b) `static` class member data is shared by all (objects),
    but owned by none. All objects of that class point to the same data at the
    same address in DATA/BSS (see below). If a static member is `public`, then it
    actually becomes truly global in scope and can be accessed anywhere, with the
    caveat that it needs to be referenced with Class::.
    As long as the program is running (main() hasn't returned), a value will
    always exist in these permanent memory addresses.

    Of course, local `static` data can only be accessed when it's within scope,
    just like regular local data. This is much safer for programmers to use
    than global data.
    Additionally, global `static` data is marked to have "internal linkage"--
    the data will be internal to its "compilation"/"translation" unit (internal
    to the file it belongs to) and can only be accessed by within that one file.
    Global data, in contrast, has "external linkage," meaning that it can be
    accessed by all files linked to the current program. This is both unsafe and
    has great potential for name collision.

    ~~Static variables will always persist.~~
    ~~"Old values see the rise and fall of generations of stack frames from their
    permanent addresses"~~
    ~~Static and global data are initialized at runtime, even if they're only
    declared (in which case it will be initialized to 0)~~
    ~~`static` means either "permanent storage" (local or member data-level) or
    "internal linkage"~~

    Details of implementation--compiler and processor specific, but generally:
    The compiler assigns *permanent* RAM addresses to global & static variables
    at compile time, creating machine code instructions for the processor to
    initialize each address with each variable's initial value as written in
    source code. In particular, these permanent RAM addresses will lie in special
    segments of RAM: "DATA" (for globals and `static`s initialized in source
    code) and BSS (for globals and `static`s declared but not initialized in
    source code, so will be initialized to 0).
    Furthermore, these initialization instructions are executed before main()--
    i.e. all global and `static` data are initialized at the start of the
    program.
    Specifically for many modern microcontrollers like Atmel AVR, `static` and
    global data will be initially stored not in SRAM but rather in Flash memory,
    where the actual program is also stored. Flash memory is non-volatile
    read/write, meaning that data within it is not erased when the power is off,
    unlike RAM. For faster processing of this `static` and global data, at
    runtime *it will be COPIED TO RAM*, into DATA/BSS.
    (This is different from most modern PCs, where instructions are stored in the
    hard drive but loaded into RAM when run).
    All variable data in C++ is stored in RAM by default-it's just that variables
    with different modifiers and scopes and allocations will be stored in
    different areas of RAM and by different means.
    */

  // Define a custom type, whose enumerated values are the size of `char`s
  enum class Mode : char
  {
    SETTING_TIME,
    SETTING_SPEED,
    SPINNING

  }; // enum class Mode

  static Mode currentMode = Mode::SETTING_TIME;         // initialization over assignment; set SETTING_TIME as the first mode!

  /* Perform operations with data of the same type, will boost processing speed
    by removing implicit typecasts, but if datatype is not the smallest that can
    hold the maximum value of that data, then this comes at a cost to SRAM.
    */

  static unsigned long setDuration;                     // measured in ms so unsigned long is appropriate
  static unsigned long spinningStartTime;               // point in time when countdown starts i.e. when wpb pressed the 2nd time
  static uint8_t motorSpeed;                            // uint8_t == typedef for an unsigned 8-bit integer aka a char aka a byte
  static bool changedUIString = false;                  // for one-time initialization of UI for certain modes

  switch(currentMode)
  {

    case Mode::SETTING_TIME:
    {

      // Set mode-specific UI only once
      if(!changedUIString)
      {
        lcd.setCursor(0, 1);
        lcd.print(F("Set time: "));                     // "Finished in:" => "Set time:"
        lcd.setCursor(11, 3);
        lcd.print(F("Start!"));                         // "Push to Stop" => "Push to Start"
        changedUIString = true;

        /* This block becomes unreachable after this, because this true value is
          remembered; therefore block ONLY DONE ONCE, when case first entered!
          */

      } // if(!changedUIString)

      setDuration = map(analogRead(POT_PIN), 0, 1024, 0, 901);

      // "Set time: <15:00>"
      FSecsPrinter fsecs(setDuration);
      surroundPrint(fsecs, UI_STATE::SELECTING, 10, 1);

      if(wpb.pressed())
      {
        /* then implicitly set in stone the countdown value, change display,
          and enter next block.
          Because mode will change, countdown will no longer be updated by pot,
          so on switch, countdown will equal its final value, no updating
          necessary.
          */

        // Erase "<>" selector braces
        surroundPrint(fsecs, UI_STATE::DESELECTING, 10, 1);

        setDuration *= 1000;                            // convert mapped time to milliseconds

        changedUIString = false;                        // set back to `false` for initializing Mode::SPINNING UI later
        currentMode = Mode::SETTING_SPEED;

      } // if(wpb.pressed())

      break;

    } // case Mode::SETTING_TIME


    case Mode::SETTING_SPEED:
    {

      motorSpeed = map(analogRead(POT_PIN), 0, 1024, 0, 255);

      // "Set speed: <255>"
      NormalPrinter normal(motorSpeed);
      surroundPrint(normal, UI_STATE::SELECTING, 11, 2);
      //map printing of motorSpeed to rpm range?!

      if(wpb.pressed())
      {
        /* then implicitly set in stone the motorSpeed value, change display,
          pre-start the countdown, and switch block
          */

        // Erase "<>" selector braces
        surroundPrint(normal, UI_STATE::DESELECTING, 11, 2);

        spinningStartTime = millis();
        currentMode = Mode::SPINNING;

      } // if(wpb.pressed())

      break;

    } // case Mode::SETTING_SPEED


    case Mode::SPINNING:
    {

      // Set mode-specific UI only once
      if(!changedUIString)
      {
        /* analogWrite() uses PWM to emulate an analog Voltage output; to do
          this, analogWrite() uses Timer1, which would cause a conflict with any
          user-defined Timer1 settings. analogWrite() has precedence over
          user-defined settings, so it would completely overwrite a custom
          Timer1.
          */

        // set motorSpeed once and you're done--the voltage stays there!
        analogWrite(MOTOR_PIN, motorSpeed);

        lcd.setCursor(0, 1);
        lcd.print(F("Finished in: "));
        lcd.setCursor(11, 3);
        lcd.print(F("Stop! "));                         // "Push to Start" => "Push to Stop"

        changedUIString = true;

      } // if(!changedIUIString)

      // timeLeft in secs for more consistent outward behavior--truncate ms
      const unsigned int timeLeft =
      (setDuration - (millis() - spinningStartTime))/1000;

      // Print countdown
      FSecsPrinter fsecs(timeLeft);
      surroundPrint(fsecs, UI_STATE::DESELECTING, 13, 1);

      if(wpb.pressed() || timeLeft == 0)
      {
        digitalWrite(MOTOR_PIN, LOW);
        changedUIString = false;                        // UI string will be changed back in case Mode::SETTING_TIME!
        currentMode = Mode::SETTING_TIME;

      } //if(wpb.pressed())

      break;

    } // case Mode::SPINNING

  } // switch(mode)

} // void loop()