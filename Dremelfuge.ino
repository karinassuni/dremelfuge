/*
Dremelfuge ETC Project 2015-2016
Adapted from Orlov's Instructable on Arduino Powered Centrifuge

Compiled and Edited by: Albert Ju, Fu Yang Chin
*/

#include <LiquidCrystal.h>                              // include the library for the display;
                                                        // because it's inside of Arduino.app use <> DIRECTIVE
#include "LEDButton.h"                                  // include custom library which is in the same directory as this file
                                                        // because it's a relative path, use "" LITERAL PATHNAME

//============================GLOBAL VARIABLES=================================//
/* *** These variables MUST be global because they're being used by multiple
  totally separate functions--static local won't do
  i.e., this data NEEDS to be available to ALL of the program at ALL times
  */
/* *** The following anonymous namespace ensures internal linkage of all global
  variables and functions, as if all were declared `static` (see large comment
  in loop() for details on `static`)
  */

namespace
{
    LiquidCrystal lcd
    = LiquidCrystal(12, 11, 5, 4, 3, 6);                  // initialize LC object with the numbers of the interface pins

    // White Pushbutton
    const uint8_t WPB_IN_PIN = 2, WPB_LED_PIN = 13;
    const unsigned int WPB_DEBOUNCE_DELAY = 500;
    LEDButton wpb
    = LEDButton(WPB_IN_PIN, WPB_LED_PIN, WPB_DEBOUNCE_DELAY);

    const uint8_t MOTOR_PIN = 9;
    const uint8_t POT_PIN = A5;


    //Function prototypes
    /* It's good practice to define the prototypes yourself, rather than letting
      the compiler auto-generate them for you
      */
    inline void lcdPrintFormattedSecs(unsigned long seconds);
} // namespace

/////////////////////////////////////////////////////////////////////////////////
void setup()
{
  const uint8_t LCD_COLUMNS = 20, LCD_ROWS = 4;

  wpb.begin();
  wpb.turnLEDOn();

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);

  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  /* On `F()`, which is a macro rather than a function, and contants/literals:
    All literals, at least in C, are obviously constants--their values can't be
    changed during program execution--and so in the programming sense are
    unnamed, unchangeable "variables." And, like all variables, literals are
    stored in RAM.
    This is a problem when dealing with string literals, which are const char
    arrays that can take up a lot of RAM without contributing anything to the
    program! All we want to do with string literals is print them, but they come
    at a huge RAM cost, so does that mean that we shouldn't make any MCU projects
    with text or GUI involved? No! Enter `F()`:
    `F()` is a macro for AVR MCUs that tells the compiler to store string
    literals in flash memory instead of RAM, and when it comes time to use that
    string literal in your program, the MCU will retrieve the string literals
    from Flash instead of RAM as well. Since variables are stored in RAM by
    default, memory addresses in source code are also read from RAM by default,
    so `F()` has to address both behaviors.
    `F()` saves you a ton of RAM by storing and reading string literals (and only
    string literals) in/from flash memory instead of in/from RAM!
    */

  lcd.setCursor(0, 0);
  lcd.print(F("  Dremel Centrifuge"));
  lcd.setCursor(0, 2);
  lcd.print(F("Set speed:"));
  lcd.setCursor(0, 3);
  lcd.print(F("   Push to Start!"));

  Serial.begin(9600);
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

  enum class Mode
  {
    SETTING_TIME,
    SETTING_SPEED,
    SPINNING
  };
  static Mode mode = Mode::SETTING_TIME;                // initialization over assignment; set SETTING_TIME as the first mode!
  /* Perform operations with data of the same type, to
    boost performance by not needing to implicitly typecast
    */
  static unsigned long setDuration;
  static unsigned long spinningStartTime;               // point in time when countdown starts i.e. when wpb pressed the 2nd time
  static uint8_t motorSpeed;

  switch(mode)
  {
    case Mode::SETTING_TIME:
    {
      static bool changedUIString = false;              // because changedUIString is static, its state will always persist
      if(!changedUIString)
      {
        lcd.setCursor(0, 1);
        lcd.print(F("Set time: <"));
        changedUIString = true;
        /* This block becomes unreachable after this, because this true value is
          remembered; therefore block only done once, when case first entered
          */
      } // if(!changedUIString)

      setDuration = map(analogRead(POT_PIN),
                        0, 1024, 0, 901);

      lcd.setCursor(11, 1);
      lcdPrintFormattedSecs(setDuration);
      lcd.print(F(">   "));

      if(wpb.pressed())
      {
        /* then implicitly set in stone the countdown value, change display,
          and enter next block.
          Because mode will change, countdown will no longer be updated by pot, so
          on switch, countdown will equal its final value, no updating necessary
          */

        // Erase <> selector braces:
        lcd.setCursor(10, 1);
        lcdPrintFormattedSecs(setDuration);
        lcd.print(F("    "));

        /* To save space, the ATmega328P chip of UNO uses 2-byte ints, which can
          hold at max 32 767; operator*(int, int) will still return an int, but in
          this case an overflowed int; initSetTime, a long, will just be assigned
          the overflowed value--it won't force operator*(int, int) to return a long.
          so, by typecasting 1000 or using 1000L, you make the compiler promote
          analogRead() to a long, so operator*(long, long) will return the expected
          value, within range of sizeof long
          */

        setDuration*= (long)1000;

        changedUIString = false;
        mode = Mode::SETTING_SPEED;
      } // if(wpb.pressed())
      break;
    } // case Mode::SETTING_TIME


    case Mode::SETTING_SPEED:
    {
      motorSpeed = map(analogRead(POT_PIN),
                       0, 1024, 0, 255);

      lcd.setCursor(11, 2);
      lcd.print(F("<"));
      //map printing of motorSpeed to rpm range?!
      lcd.print(motorSpeed);
      lcd.print(F(">  "));

      if(wpb.pressed())
      {
        /* then implicitly set in stone the motorSpeed value, change display,
          pre-start the countdown, and switch block
          */

        // Erase <> selector braces:
        lcd.setCursor(11, 2);
        lcd.print(motorSpeed);
        lcd.print(F("  "));

        spinningStartTime = millis();

        mode = Mode::SPINNING;
      } // if(wpb.pressed())
      break;
    } // case Mode::SETTING_SPEED


    case Mode::SPINNING:
    {
      static bool changedUIString = false;
      if(!changedUIString)
      {
        analogWrite(MOTOR_PIN, motorSpeed);             // set motorSpeed once and you're done--the voltage stays there!

        lcd.setCursor(0, 1);
        lcd.print(F("Finished in: "));
        changedUIString = true;
        Serial.println("reached only once");
      } // if(!changedIUIString)

      unsigned long secondsLeft =                       // save RAM by only doing this calculation only once per loop()::SPINNING
      (setDuration - (millis() - spinningStartTime))/1000;

      lcd.setCursor(13, 1);
      lcdPrintFormattedSecs(secondsLeft);
      lcd.print(F("  "));

      if(wpb.pressed() || secondsLeft == 0)
      {
        digitalWrite(MOTOR_PIN, LOW);
        changedUIString = false;                        // UI string will be changed back!
        mode = Mode::SETTING_TIME;
      } // if(wpb.pressed() || secondsLeft == 0)
      break;
    } // case Mode::SPINNING
  } // switch(mode)
} // void loop()

/* `inline` = suggestion to the compiler to paste the raw code of this function
  wherever it's called == performance optimization, no hopping pointers to callers
  and callbacks
  */
inline void lcdPrintFormattedSecs(unsigned long seconds)
{
  lcd.print(seconds/60); lcd.print(F(":")); lcd.print((seconds%60 < 10 ? F("0") : F(""))); lcd.print(seconds%60);
}
