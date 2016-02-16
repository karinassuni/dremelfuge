#include <Arduino.h>

struct PercentPrint {

    template <typename Number, class Stream>
    void operator() (const Number value, Stream& stream) {

        stream.print(value);
        stream.print(F("%"));

    }

};

struct FSecsPrint {

    template <typename Number, class Stream>
    void operator() (const Number seconds, Stream& stream) {

        stream.print(seconds/60);
        stream.print(F(":"));
        stream.print((seconds%60 < 10 ? F("0") : F("")));
        stream.print(seconds%60);

    }

};