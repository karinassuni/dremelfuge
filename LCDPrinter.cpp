#include "LCDPrinter.h"

void LCDPrinter::changeLine(const char* string, uint8_t line) {

  setCursor(0, line);
  print(string);

}

void LCDPrinter::changeLine_P(PGM_P string, uint8_t line) {

  setCursor(0, line);
  print_P(string);

}

void LCDPrinter::print_P(PGM_P string) {
  
  // Addresses in Flash are read 1 byte or word at a time; so print the same way
  for(uint8_t i = 0; i < strlen_P(string); i++)

    // Need to typecast to char to print a character rather than an ASCI code
    print( (char) pgm_read_byte(string + i));

}

void LCDPrinter::printAt(const char* string, uint8_t col, uint8_t row) {

  setCursor(col, row);
  print(string);

}

void LCDPrinter::printAt_P(PGM_P string, uint8_t col, uint8_t row) {

  setCursor(col, row);
  print_P(string);

}