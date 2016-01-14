// Purpose for creating this library:
  // To replace similar, repetitive lines for printing with DRY functors!
  
#ifndef Printers
#define Printers
#include <Arduino.h>

// Functors that print values in specialized ways; plugged into Printer, used
// primarily as caller-determined customizers for decorationPrint()

struct NormalPrint {

  template <typename T, class Stream>
  void operator() (const T value, Stream* streamPtr) {
    streamPtr->print(value);
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
    void printfval_P(const T value, Functor& printfn, PGM_P decorStr, const uint8_t col, const uint8_t row) {

      // Note: decorStr must be a of length 2, such as "<>"; could be blank

      streamPtr->setCursor(col, row);

      // If not blank
      if(decorStr)
        this->print_P(&decorStr[0]);

      printfn(value, streamPtr);

      // If not blank
      if(decorStr)
        this->print_P(&decorStr[1]);

      // Clear hanging digits
      streamPtr->print(F("   "));

    } // void printfval

    inline void changeLine_P(PGM_P string, uint8_t line) {

      streamPtr->setCursor(0, line);
      streamPtr->print( (PGM_P) pgm_read_word(&string));

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

    } // void changeLine_P

    inline void print_P(PGM_P string) {
      streamPtr->print( (PGM_P) pgm_read_word(&string));
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