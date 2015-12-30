/* Although it's redundant for the ~functions~ being defined in this file, the
  #include of the header ensures that any special type declarations within it
  will be usable by this file, *such as Arduino's special String class*
  */
#include "karina_utility.h"
#include <Arduino.h>

inline String millisToFSecs(const long& millis, const int& numDecimals)
{
  const int divisor = pow(10, numDecimals-1);                                    // `^` = bitwise OR
  return String(millis/1000) + "." + String(millis%1000/divisor);                // operator+(String, int) is invalid
  //1.532s = "1." + (1532%1000 = 532)/(10 = 10^(2-1) = divisor) = "53"
}