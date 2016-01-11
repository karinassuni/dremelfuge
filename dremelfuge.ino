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

// 1) Mode structs in PROGMEM, Mode::id in PROGMEM
// 2) constexpr function and named variables for timeIndex and speedIndex, to be put
// in printfval

namespace
{
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

  // UI Modes - Each Mode is associated with UI Strings describing its UI
  struct Mode {

    /* `PROGMEM` is a macro for an attribute that tells the compiler to write
      instructions for loading this variable into Flash memory at the start of
      runtime.
      `PGM_P` is a macro for the type const char* (pointer to (the start of) a
      string), useful since it connotates that this variable is stored in Flash,
      which clarifies for the user and helps the compiler optimize assembly code.
      */
    
    // Store array of 4 pointers to strings in Flash memory
    // Note: strings to which pointers point aren't restricted in length
    PGM_P UIStringPtrs[LCD_ROWS] PROGMEM;
    const uint8_t id;

    // Tie integer row indicies to meaningful names based on information layout
    enum UIStringType : char {
      Title = 0,
      Time,
      Speed,
      Instruction
    };

    // Tie UI Modes with integer IDs so that `switch` works--only accepts ints
    enum ID : char {
      SETTING_TIME,
      SETTING_SPEED,
      SPINNING
    };

  }; // struct Mode

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
    
  // Static (not the keyword) UI elements + UI associated with setting up Modes
  const char titleStr[] PROGMEM      = "  Dremel Centrifuge";
  const char setTimeStr[] PROGMEM    = "Set time: ";
  const char setSpeedStr[] PROGMEM   = "Set speed: ---";
  const char startStr[] PROGMEM      = "   Push to Start!";

  // UI associated with SPINNING mode
  const char finishedInStr[] PROGMEM = "Finished in: ";
  const char stopStr[] PROGMEM       = "   Push to Stop! ";
  const char null_str[] PROGMEM      = "";

} // namespace

/////////////////////////////////////////////////////////////////////////////////

void setup()
{  
  lcd.begin(LCD_COLUMNS, LCD_ROWS);

  wpb.begin();
  wpb.turnLEDOn();

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);

  // Motor is OFF on startup
  digitalWrite(MOTOR_PIN, LOW);

  // Associate static elements of LCD UI with a temporary Mode
  Mode setupMode = {
    .UIStringPtrs =
    {
      titleStr,  // accessing a whole array, so its (starting) address is returned
      null_str,
      setSpeedStr,
      null_str
    },
    .id = 0
  };

  // Initialize static (not the keyword) elements of LCD UI
  for(int row = 0; row < LCD_ROWS; row++) {

    lcd.setCursor(0, row);

    /* `pgm_read_word` implementation:
      Implements a Load Program Memory instruction on an 8-bit RAM
      address argument that's converted to a 16-bit Flash memory address before
      the address is read and has its stored value returned.
      http://www.atmel.com/images/doc1233.pdf
      http://www.atmel.com/webdoc/AVRLibcReferenceManual/group__avr__pgmspace_1ga7fa92c0a662403a643859e0f33b0a182.html

      Need `&` because it's not just the char arrays[] that's stored in Flash,
      but the pointers to these strings, held in UIStringPtrs, are stored in
      Flash too. So, convert the pointer and then return its Flash data.
      */

    // Read and the UIStringPtrs from Flash memory, and print
    lcd.print( (PGM_P) pgm_read_word(&(setupMode.UIStringPtrs[row])));

  } // for loop LCD_ROWS

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

  // Alias for the verbose Mode::UIStringType type name
  typedef Mode::UIStringType line;

  // Functor that encapsulates (hides the implementation of) changing UI ONCE
  // per the current mode's own UIStringPtrs; extension of LiquidCrystal:

  struct UIChanger {

    private:

      // Remember state of (re)initialization, so UI changes ONCE per case switch
      bool initialized;

      // Remember which LiquidCrystal object to operate on
      LiquidCrystal* lcdPtr;

    public:

      // Initialization of state
      UIChanger(LiquidCrystal* dPtr)
      : lcdPtr(dPtr)
      , initialized(false)
      {}

      // Initialization of UI based on current Mode
      void operator() (Mode* currentModePtr) {

        // Don't initialize UI more than once per case entry
        if(initialized)
          return;

        // Change UI based on currentMode's own UIStringPtrs, read from Flash memory
        // Partial overwrite due to overwriteStrs that do not span the entire row

        // "Set time:" => "Finished in:"
        lcdPtr->setCursor(0, line::Time);
        lcdPtr->print( (PGM_P) pgm_read_word(&(currentModePtr->UIStringPtrs[line::Time])));

        // "Push to Start!" => "Push to Stop!"
        lcdPtr->setCursor(0, line::Instruction);
        lcdPtr->print( (PGM_P) pgm_read_word(&(currentModePtr->UIStringPtrs[line::Instruction])));

        initialized = true;

      }

      // Uninitialize so that UI can be initialized only once for another case
      void uninitialize() {
        initialized = false;
      }

  };

  // All static--initialize structs only once, before main()

  static Mode setTimeMode = {
    .UIStringPtrs =
    {
      null_str,
      setTimeStr,
      null_str,
      startStr
    },
    .id = Mode::ID::SETTING_TIME
  };

  static Mode setSpeedMode = {
    .UIStringPtrs =
    {
      null_str,
      null_str,
      null_str,
      null_str
    },
    .id = Mode::ID::SETTING_SPEED
  };

  static Mode spinningMode = {
    .UIStringPtrs =
    {
      null_str,
      finishedInStr,
      null_str,
      stopStr
    },
    .id = Mode::ID::SPINNING
  };

  // Initialize setTimeMode as the starting Mode
  // Current Mode kept in a pointer so that entire struct needn't be copied
  static Mode* currentModePtr = &setTimeMode;

  // Extension of LiquidCystal as a Stream
  static Printer<LiquidCrystal> lcdPrinter(&lcd);

  // Initialize UIChanger functor for use in each case-also extends LiquidCrystal
  static UIChanger changeUI(&lcd);  

  // Functors customizing methods on lcdPrinter
  static NormalPrint raw;                               // Default constructors
  static FSecsPrint fsecs;

  /* Perform operations with data of the same type, will boost processing speed
    by removing implicit typecasts, but if datatype is not the smallest that can
    hold the maximum value of that data, then this comes at a cost to SRAM.
    */

  static unsigned long setDuration;                     // measured in ms so unsigned long is appropriate
  static unsigned long spinningStartTime;               // point in time when countdown starts i.e. when wpb pressed the 2nd time
  static uint8_t motorSpeed;                            // uint8_t == typedef for an unsigned 8-bit integer aka a char aka a byte

  switch(currentModePtr->id) {

    case Mode::ID::SETTING_TIME: {

      // Initialize UI only once per case entry; state of initialization
      // remembered due to functor being holding state and being `static`
      changeUI(currentModePtr);

      // Map time value from potentiometer
      setDuration = map(analogRead(POT_PIN), 0, 1024, 0, 901);

      // Add selector braces: "Set time: <15:00>"
      lcdPrinter.formatValue(setDuration, fsecs, ValueDecor::SELECTING,
                             10, line::Time);

      if(wpb.pressed())
      {

        // Erase "<>" selector braces
        lcdPrinter.formatValue(setDuration, fsecs, ValueDecor::DESELECTING,
                               10, line::Time);

        // Convert mapped time to calculatable millis after printing from seconds
        setDuration *= 1000;                            

        // Change the mode for next loop() call
        currentModePtr = &setSpeedMode;

        // Reset functor state so that another Mode's UI can be initialized once
        changeUI.uninitialize(); 

      } // if(wpb.pressed())

      break;

    } // case Mode::SETTING_TIME

    case Mode::ID::SETTING_SPEED: {

      // Map motor speed to potentiometer
      motorSpeed = map(analogRead(POT_PIN), 0, 1024, 0, 255);

      // Add selector braces: "Set speed: <255>"
      lcdPrinter.formatValue(motorSpeed, raw, ValueDecor::SELECTING,
                             11, line::Speed);
      // Map printing of motorSpeed to rpm range?!

      if(wpb.pressed()) {

        // motorSpeed implicitly set in stone--the only block it's able to be
        // updated in becomes unreachable

        // Erase "<>" selector braces
        lcdPrinter.formatValue(motorSpeed, raw, ValueDecor::DESELECTING,
                               11, line::Speed);

        // Change the mode for next loop() call
        currentModePtr = &spinningMode;

        // Start the counter from here, now that the mode is about to change
        spinningStartTime = millis();

      } // if(wpb.pressed())

      break;

    } // case Mode::SETTING_SPEED


    case Mode::ID::SPINNING: {

      // Change UI once
      changeUI(currentModePtr);

      // Truncate milliseconds (/1000) for more consistent displayed countdowns
      // Store in a variable so that it's calculated only once
      const unsigned long timeLeft =
      (setDuration - (millis() - spinningStartTime))/1000;

      // Print countdown (no special functions needed)
      lcd.setCursor(13, line::Time);
      lcd.print(timeLeft);

      if(wpb.pressed() || timeLeft == 0) {

        // Turn off motor
        digitalWrite(MOTOR_PIN, LOW);

        // Allow static UIChanger to reinitialize once on next entry to this case
        changeUI.uninitialize();

        // Change mode for next loop() call
        currentModePtr = &setTimeMode;

      } //if(wpb.pressed())

      break;

    } // case Mode::SPINNING

  } // switch(mode)

} // void loop()