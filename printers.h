// Purpose for creating this library:
  // To replace similar, repetitive lines for printing with DRY functors!
  
#ifndef printers
#define printers
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
};

enum class ValueDecor : char
{
  SELECTING,
  DESELECTING
};

// Specialization of a hypothetical instance of the Printer template class

template <>
class Printer<LiquidCrystal> {

  private:

    LiquidCrystal* streamPtr;

  public:

    Printer(LiquidCrystal* sPtr) : streamPtr(sPtr) {}

    template <typename T, class Functor>
    void formatValue(const T value, Functor& printfn, const ValueDecor decor, const uint8_t col, const uint8_t row) {

      streamPtr->setCursor(col, row);

      if(decor == ValueDecor::SELECTING)
        streamPtr->print(F("<"));

      printfn(value, streamPtr);

      if(decor == ValueDecor::SELECTING)
        streamPtr->print(F(">"));

      streamPtr->print(F("   "));

    }

    friend class Print;
    friend class FSecsPrint;
    
};

#endif

// Notes:
  // Make sure your libraries are separate from any of their specific implementations

  // When you call operator(), make sure it's `operator()()`!!!
  // You CAN call a Derived class-overriden virtual method from a different Base class method

  /* If error messages are super numerous, they're most likely inaccurate: make
    sure you're calling functions, specifying templates, putting brackets, and
    putting parenthesis correctly FIRST!
    */