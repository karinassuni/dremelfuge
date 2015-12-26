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
// `const` params in function DECLARATION are compatible with non-const arguments:
inline String millisToFSecs(const long& millis, const int& numDecimals);

// for types whose print() inherits from Serial.print(), e.g. LiquidCrystal:
template<typename Stream>
inline void printfSecs(const unsigned long& seconds, Stream& stream);            // `Time left: 5:56`

/////////////////////////////////////////////////////////////////////////////////

String millisToFSecs(const long& millis, const int& numDecimals)
{
  const int divisor = pow(10, numDecimals-1);                                    // `^` = bitwise OR
  return String(millis/1000) + "." + String(millis%1000/divisor);                // operator+(String, int) is invalid
  //1.532s = "1." + (1532%1000 = 532)/(10 = 10^(2-1) = divisor) = "53" + "s"
}

template<typename Stream>
void printfSecs(const unsigned long& seconds, Stream& stream)
{
  stream.print(seconds/60);
  stream.print(F(":"));
  stream.print((seconds%60 < 10 ? F("0") : F("")));
  stream.print(seconds%60);
}

} // inline namespace