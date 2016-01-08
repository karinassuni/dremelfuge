// Header files are a repository for declarations of functions and special types
/* Function prototypes/declarations tell the compiler that functions with these
  signatures do exist. During linking phase, all compiled files are combined,
  meaning that all functionality guaranteed by the declarations will end up in
  the executable. The reason that there are separate header and cpp files is that
  it is very useful, especially for large projects, for (public) interface to be
  separate from (private) implementation.
  */
/* Note that, when compiling from an IDE, all files in the project folder will be
  compiled (separately, before their .o code is all linked together) */

#ifndef karina_utility_h
#define karina_utility_h

#include <Arduino.h>

// "The following declarations belong to the 'default namespace'", no `::` needed

inline namespace
{
  /* `inline` for functions = suggestion to the compiler to paste the raw code of
    this function wherever it's called == performance optimization, no hopping
    pointers to callers and callbacks
    */
  // `inline` creates a unique signature, so the definition must also say `inline`
  // `const` params in function DECLARATION are compatible with non-const args:

  inline String millisToFSecs(const long& millis, const int& numDecimals);       // `Time left: 5:56`

  // For types whose print() inherits from Serial.print(), e.g. LiquidCrystal:
  // A "template parameter declaration" declaring `Stream` a placeholder type:

  template <typename Stream>
  inline void printFSecs(const unsigned long& seconds, Stream& stream);

  /* Template instantiation is done at compile time, so template implementation
    must be AVAILABLE at compile time as well, otherwise instantiation can't
    occur then--"I'm compiling, I've encountered the use of a template function,
    I need to know HOW to make a real function out of that RIGHT NOW so I can
    verify that the code probably works properly!" Hence the #include of .tpp,
    aka the template implementation file, which pastes the contents of that file
    RIGHT HERE during preprocessing
    */
  #include "karina_utility_T.cpp"                                                // Arduino.app won't recognize .tpp

} // inline namespace

#endif