#ifndef LCDPrinter_h
#define LCDPrinter_h

#include <Arduino.h>
#include <LiquidCrystal.h>

struct LCDPrinter : public LiquidCrystal {

    LCDPrinter(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f)
    : LiquidCrystal(a,b,c,d,e,f)
    {}

    void changeLine(const char* string, uint8_t line);

    void changeLine_P(PGM_P string, uint8_t line);

    void print_P(PGM_P string);

    void printAt(const char* string, uint8_t col, uint8_t row);

    void printAt_P(PGM_P string, uint8_t col, uint8_t row);

};

#endif