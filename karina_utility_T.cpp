#include <Arduino.h>

template <typename Stream>
inline void printFSecs(const unsigned long& seconds, Stream& stream)
{
  stream.print(seconds/60);
  stream.print(F(":"));
  stream.print((seconds%60 < 10 ? F("0") : F("")));
  stream.print(seconds%60);
} // printFSecs()