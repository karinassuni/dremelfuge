// Purpose for creating this library:
  // To replace similar, repetitive lines for printing with DRY functors!
  
#ifndef Printers
#define Printers
#include <Arduino.h>

// Functors that print values in specialized ways; plugged into Printer, used
// primarily as caller-determined customizers for decorationPrint()

struct PercentPrint {

  template <typename T, class Stream>
  void operator() (const T value, Stream* streamPtr) {

    streamPtr->print(value);
    streamPtr->print(F("%"));

  }

};

struct FSecsPrint {

  template <typename Number, class Stream>
  void operator() (const Number seconds, Stream* streamPtr) {

    streamPtr->print(seconds/60);
    streamPtr->print(F(":"));
    streamPtr->print((seconds%60 < 10 ? F("0") : F("")));
    streamPtr->print(seconds%60);

  }

};

// `Printer` is a wrapper/extension of LiquidCrystal and other Stream classes.
// Contains a pointer to the stream it extends so it can be used similarly.

template <class Stream>
class Printer {
  private:
    Stream* streamPtr;

  public:
    Printer(Stream* sPtr) : streamPtr(sPtr) {}

    friend class Print;
    friend class FSecsPrint;
}; // class Printer

// Specialization of a hypothetical instance of the Printer template class:
template <>
class Printer<LiquidCrystal> {

  private:

    LiquidCrystal* streamPtr;

  public:

    Printer(LiquidCrystal* sPtr) : streamPtr(sPtr) {}

    template <typename T, class Functor>
    void printfval(const T value, Functor& printfn, const char* decorStr, const uint8_t col, const uint8_t row) {

      // Note: decorStr must be either 2 characters long (e.g. "<>"), or be blank

      streamPtr->setCursor(col, row);

      // If VALUE OF STRING ADDRESS not blank
      if(*decorStr)
        streamPtr->print( decorStr[0] );

      printfn(value, streamPtr);

      // If VALUE OF STRING ADDRESS not blank
      if(*decorStr)
        streamPtr->print( decorStr[1] );

      // Clear hanging digits
      streamPtr->print(F("   "));

    } // void printfval

    inline void changeLine_P(PGM_P string, uint8_t line) {

      streamPtr->setCursor(0, line);

      this->print_P(string);

    } // void changeLine_P

    inline void print_P(PGM_P string) {

      uint8_t strLength = strlen_P(string);
      
      // Addresses in Flash are read 1 byte or word at a time; print accordingly
      for(uint8_t i = 0; i < strLength; i++)

        // Need to typecast to char to print a character rather than an ASCI code
        streamPtr->print( (char) pgm_read_byte(string + i));

      /* `pgm_read_byte` implementation:
        Implements a Load Program Memory instruction on an 8-bit RAM
        address argument that's converted to a 16-bit Flash memory address before
        the address is read and has its stored value returned.
        http://www.atmel.com/images/doc1233.pdf
        http://www.atmel.com/webdoc/AVRLibcReferenceManual/group__avr__pgmspace_1ga7fa92c0a662403a643859e0f33b0a182.html
        */

    }

    friend class Print;
    friend class FSecsPrint;
    
}; // class Printer<LiquidCrystal>

#endif

// Notes:

  // Make sure your libraries are separate from any of their specific implementations

  // When you call operator(), make sure it's `operator()()`!!!
  // You CAN call a Derived class-overriden virtual method from a different Base class method

  /* If error messages are super numerous, they're most likely inaccurate: make
    sure you're calling functions, specifying templates, putting brackets, and
    putting parenthesis correctly FIRST!
    */