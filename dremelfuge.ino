/*
Dremelfuge ETC Project 2015-2016
Adapted from Orlov's Instructable on Arduino Powered Centrifuge

Compiled and Edited by: Albert Ju, Fu Yang Chin

Uploaded to an Arduino UNO, with MCU ATmega328P-PU.
Arduino UNO specs: https://www.arduino.cc/en/Main/ArduinoBoardUno
ATmega328P-PU datasheet: http://www.atmel.com/Images/doc8161.pdf

Sketch uses 4,408 bytes (13%) of program storage space. Maximum is 32,256 bytes.
Global variables use 82 bytes (4%) of dynamic memory. Maximum is 2,048 bytes.
*/

#include <LiquidCrystal.h>                              // library inside of Arduino.app, use <> DIRECTIVE
#include "LEDButton.h"                                  // library not in Arduino.app but in sketch folder, use "" LITERAL PATHNAME
#include "Printers.h"                                   // wrapper/extension class of LiquidCrystal and other Stream classes
#include <avr/pgmspace.h>                               // contains macros for storing data in Flash instead of RAM, and fetching it 

/*============================GLOBAL VARIABLES=================================//
  These variables MUST be global because they're being used by multiple
  totally separate functions--static local won't do
  i.e., this data NEEDS to be available to ALL of the program at ALL times
  */
/* The following anonymous namespace ensures internal linkage of all global
  variables and functions, as if all globals were declared `static` (see large
  comment in loop() for details on `static`)
  */

namespace {

  const uint8_t MOTOR_PIN = 9;
  const uint8_t POT_PIN = A5;

  // White PushButton
  const uint8_t WPB_IN_PIN = 2;
  const uint8_t WPB_LED_PIN = 13;
  const unsigned int WPB_DEBOUNCE_DELAY = 500;
  LEDButton wpb = LEDButton(WPB_IN_PIN, WPB_LED_PIN, WPB_DEBOUNCE_DELAY);        // cannot be `const`, member data will change

  // LCD
  LiquidCrystal lcd = LiquidCrystal(12, 11, 5, 4, 3, 6);                         // numbers of the interface pins
  const uint8_t LCD_COLUMNS = 20;
  const uint8_t LCD_ROWS = 4;

  // Extension of LiquidCystal as a Stream
  Printer<LiquidCrystal> lcdPrinter(&lcd);

  // Functors customizing methods on lcdPrinter
  NormalPrint raw;               // Default constructors
  FSecsPrint fsecs;

  /* Explanation of string problem and contants/literals:
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
    shouldn't make any MCU projects with text or GUI involved? No! There are
    macros in avr/pgmspace.h for storing data in Flash (Program) memory and
    retrieving it when needed.
    */

  // UI Strings to be stored in Flash memory *as arrays of characters*
    /* Arrays MUST be uniform, and this is accomplished by either a uniform 2D
      array of chars, or if that's too inefficient due to too much variation in
      string length, accomplished by a 1D array of pointers to any-length
      strings (char arrays[] stored as variables).
      The latter can be inefficient since you're storing and retrieving in Flash
      an additional array, of pointers.
      */

  namespace UI {

    /* Explanation of `constexpr`:
      `constexpr` stands for "constant expression" and denotes that this variable
      will have its value accessible at compile time, or denotes that this
      function (which has several limitations, including that its arguments can
      only be constexpr values) can be used to do computations at compile-time.
      Note that literals are constant expressions.
      */

    // Use constexpr here to obtain AT COMPILE TIME indicies that are important
    // for runtime UI, without hard-coding the appropriate magic index. Besides
    // clarity, compile time computation has two other huge benefits: program
    // storage space (Flash memory) is saved since machine instructions aren't
    // generated for these one-time calculations (since after compile time the
    // instructions are already executed and the results obtained), AND RAM is
    // saved because, for modified Harvard-architecture MCUs like Atmel AVRs,
    // read only memory (ROM) is stored separately (in Flash) from dynamic memory
    // (RAM), and might even be accessed faster. Additionally, here compile-time
    // computation is necessary because these UI strings can be operated on
    // before they are placed in Flash.

    constexpr char title[] PROGMEM      = "  Dremel Centrifuge";
    constexpr char setTime[] PROGMEM    = "Set time: ";
    constexpr char setSpeed[] PROGMEM   = "Set speed: ";
    constexpr char pushStart[] PROGMEM  = "   Push to Start!";

    // UI associated with SPINNING mode
    constexpr char finishedIn[] PROGMEM = "Finished in: ";
    constexpr char pushStop[] PROGMEM   = "   Push to Stop! ";
    constexpr char nullValue[] PROGMEM  = "---";
    constexpr char selected[] PROGMEM = "<>";
    constexpr char deselected[] PROGMEM = "";

    constexpr uint8_t length(const char* string)
    {
      return *string ? 1 + length(string + 1) : 0;
    }

    constexpr uint8_t setTimeIndex = length(setTime) - 1;
    constexpr uint8_t speedIndex = length(setSpeed) - 1;
    constexpr uint8_t finishTimeIndex = length(finishedIn) - 1;    

  } // namespace UI

  /* char var_name[] vs char* var_name:
    `char var_name[]` is an ARRAY of characters, which is different from `char*`
    which is a POINTER TO A CHAR. The confusion arises because whenever arrays
    are passed, it's the address of the array's first element that's passed,
    rather than all of its contents (this is why you need the length of an array
    separately when iterating over an array--the only data that's passed with
    arrays is its first element's, not its length or contents).

    A char* is a pointer, so it can be initialized with the address of an
    individual `char`, especially the address of (the first element of) an array
    of chars. Since string literals are of type `const char[length]` (an array of
    `char`s), you can also initialize `char*`s with literals.

    Even though passed arrays are actually just char* s, the two datatypes are
    different.

    Note that for arrays, when no number is found between the braces in its
    declaration (e.g. char array[] = "123";), the size of the array is implicitly
    determined based on the number of elements, and only the minimum amount of
    memory to store the full array is allocated.
    */  

  // Enumeration of row indices as UI elements
  enum line : char
  {
    Title = 0,
    Time,
    Speed,
    Instructions
  };

} // namespace

// Application-specific function: Time and Instructions are the only dynamic UI
inline void changeUI(PGM_P timeStr, PGM_P instructionStr) {

  // "Set time:" <=> "Finished in:"
  lcdPrinter.changeLine_P(timeStr, line::Time);

  // "Push to Start!" <=> "Push to Stop!"
  lcdPrinter.changeLine_P(instructionStr, line::Instructions);

} // void changeUI

/////////////////////////////////////////////////////////////////////////////////

void setup() {

  lcd.begin(LCD_COLUMNS, LCD_ROWS);

  wpb.begin();
  wpb.turnLEDOn();

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);

  // Motor is OFF on startup
  digitalWrite(MOTOR_PIN, LOW);

  // Read and print the UIStrings from Flash memory
  using namespace UI;
  lcdPrinter.changeLine_P(title, line::Title);
  lcdPrinter.changeLine_P(setTime, line::Time);
  lcdPrinter.changeLine_P(setSpeed, line::Speed);
  lcdPrinter.print_P(nullValue);
  lcdPrinter.changeLine_P(pushStart, line::Instructions); 

} // void setup()

/////////////////////////////////////////////////////////////////////////////////

void loop() {

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
  };

  static Mode currentMode = Mode::SETTING_TIME;         // initialization over assignment; set SETTING_TIME as the first mode!

  /* Perform operations with data of the same type, will boost processing speed
    by removing implicit typecasts, but if datatype is not the smallest that can
    hold the maximum value of that data, then this comes at a cost to SRAM.
    */

  static unsigned long setDuration;                     // measured in ms so unsigned long is appropriate
  static unsigned long spinningStartTime;               // point in time when countdown starts i.e. when wpb pressed the 2nd time
  static uint8_t motorSpeed;                            // uint8_t == typedef for an unsigned 8-bit integer aka a char aka a byte

  switch(currentMode) {

    case Mode::SETTING_TIME: {

      // Map time value from potentiometer--max is 15 minutes
      setDuration = map(analogRead(POT_PIN), 0, 1024, 0, 901);

      // Add selector braces: "Set time: <15:00>"
      lcdPrinter.printfval_P(setDuration, fsecs, UI::selected,
                            UI::setTimeIndex, line::Time);

      if(wpb.pressed()) {

        // Overwrite "<>" selector braces
        lcdPrinter.printfval_P(setDuration, fsecs, UI::deselected,
                              UI::setTimeIndex, line::Time);

        // Convert mapped time to calculatable millis after printing from seconds
        setDuration *= 1000;                            

        // Change the mode for next loop() call
        currentMode = Mode::SETTING_SPEED;

      } // if(wpb.pressed())

      break;

    } // case Mode::SETTING_TIME


    case Mode::SETTING_SPEED: {

      // Map motor speed to potentiometer
      motorSpeed = map(analogRead(POT_PIN), 0, 1024, 0, 255);

      using namespace UI;

      // Add selector braces: "Set speed: <255>"
      lcdPrinter.printfval_P(motorSpeed, raw, UI::selected,
                            UI::speedIndex, line::Speed);
      // Map printing of motorSpeed to rpm range?!

      if(wpb.pressed()) {

        // motorSpeed implicitly set in stone--the only block it's able to be
        // updated in becomes unreachable

        // Overwrite "<>" selector braces
        lcdPrinter.printfval_P(motorSpeed, raw, UI::deselected,
                              UI::speedIndex, line::Speed);

        // Change the mode for next loop() call
        currentMode = Mode::SPINNING;

        // Initialize the UI for SPINNING Mode (note: only done once, this case
        // becomes unreachable due to changing the Mode pointer!)
        changeUI(UI::finishedIn, UI::pushStop);

        // Turn on motor right before the countdown starts, for more accuracy
        analogWrite(MOTOR_PIN, motorSpeed);

        // Start the counter from here, now that the mode is about to change
        spinningStartTime = millis();

      } // if(wpb.pressed())

      break;

    } // case Mode::SETTING_SPEED


    case Mode::SPINNING: {

      // Truncate milliseconds (/1000) for more consistent displayed countdowns
      // Store in a variable so that it's calculated only once
      const unsigned long timeLeft =
      (setDuration - (millis() - spinningStartTime))/1000;

      // Print countdown (no special functions needed)
      lcd.setCursor(UI::finishTimeIndex, line::Time);
      lcd.print(timeLeft);

      if(wpb.pressed() || timeLeft == 0) {

        // Turn off motor
        digitalWrite(MOTOR_PIN, LOW);

        // Change mode for next loop() call
        currentMode = Mode::SETTING_TIME;

        // Initialize the UI for SETTING_TIME Mode (note: only done once, here!)
        changeUI(UI::setTime, UI::pushStart);

      } //if(wpb.pressed())

      break;

    } // case Mode::SPINNING

  } // switch(mode)

} // void loop()