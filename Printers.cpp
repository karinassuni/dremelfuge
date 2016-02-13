#include <Arduino.h>

struct PercentPrint {

  void operator() () {}

  template <typename Number, class Stream>
  void operator() (const Number value, Stream* streamPtr) {

    streamPtr->print(value);
    streamPtr->print(F("%"));

  }

};

struct FSecsPrint {

  void operator() () {}

  template <typename Number, class Stream>
  void operator() (const Number seconds, Stream* streamPtr) {

    streamPtr->print(seconds/60);
    streamPtr->print(F(":"));
    streamPtr->print((seconds%60 < 10 ? F("0") : F("")));
    streamPtr->print(seconds%60);

  }

};