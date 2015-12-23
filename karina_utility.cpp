#include <Arduino.h>

/* `inline` anonymous namespace = "this namespace and all of its sub-namespaces'
  variables and functions are the default variables and functions that are
  referred to whenever their names are used," i.e. it's as if you're writing
  `using namespace thisunnamedanonymousenamespace;`
  */

inline namespace
{
  /* `inline` for functions = suggestion to the compiler to paste the raw code of
    this function wherever it's called == performance optimization, no hopping
    pointers to callers and callbacks
    */

  inline String millisToFSecs(long millis, int numDecimals)
  {
    const int divisor = pow(10, numDecimals-1);         //*** ^ is the bitwise OR operator, so 10^(numDecimals-1) won't work
    return String(millis/1000) + "."
    + String(millis%1000/divisor);

    /* return String("." + (millis%1000)/divisor) is bugged; i.e., adding a
      string literal and an int is not trivial in C++ and cannot be done directly
      through operator+

      call(1532 ms, 2 decimals) = 1.532 s = "1" + "." + **(remainder of 1532/1000 = 532)/(10 = 10^(2-1) = divisor) = "53"** + "s"
      */
  } // String millisToFSecs()

  template<typename T>                                  // meant for types whose print() inherits from Serial.print()
  inline void printfSecs(unsigned long seconds, T& t)   // typename T will take care of using LiquidCrystal lcd, or Serial
  {
    t.print(seconds/60); t.print(F(":"));
    t.print((seconds%60 < 10 ? F("0") : F("")));
    t.print(seconds%60);
  } // void printfSecs()



} // inline namespace