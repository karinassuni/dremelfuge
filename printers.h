// Replace similar, repetitive lines for printing with DRY functors!
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
    void decorationPrint(const T value, Functor printfn, const ValueDecor decor, const uint8_t col, const uint8_t row) {
      streamPtr->setCursor(col, row);

      if(decor == ValueDecor::SELECTING)
        streamPtr->print(F("<"));

      printfn(value, streamPtr);

      if(decor == ValueDecor::SELECTING)
        streamPtr->print(F(">"));

      streamPtr->print(F("   "));
    }

    void replaceRow(char rowToReplace, const char* replacementStrs) {
      streamPtr->setCursor(0, rowToReplace);
      streamPtr->print(replacementStrs[rowToReplace]);
    }
    void replace(char rowToReplace, char colToReplace, const char* replacementStrs) {
      streamPtr->setCursor(colToReplace, rowToReplace);
      streamPtr->print(replacementStrs[rowToReplace]);
    }

    friend class Print;
    friend class FSecsPrint;
};

#endif