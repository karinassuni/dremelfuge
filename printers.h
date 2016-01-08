// Replace similar, repetitive lines for printing with DRY functors!

class FSecsPrinter {
  public:
    FSecsPrinter(unsigned long v) : value(v) {}
    void operator()() {
      printFSecs(value, lcd);
    }
  private:
    unsigned long value;
};

class NormalPrinter  {
  public:
    NormalPrinter(uint8_t v) : value(v) {}
    void operator()() {
      lcd.print(value);
    }
  private:
    uint8_t value;
};

enum class UI_STATE : char {
  SELECTING,
  DESELECTING
};

template <typename Printer>
void surroundPrint(Printer& printValue, const UI_STATE& ui, const uint8_t& col, const uint8_t& row) {
  lcd.setCursor(col, row);
  if(ui == UI_STATE::SELECTING)
    lcd.print(F("<"));
  printValue();
  if(ui == UI_STATE::SELECTING)
    lcd.print(F(">"));
  lcd.print(F("   "));
}