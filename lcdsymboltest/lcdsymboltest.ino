#include <LiquidCrystal.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0xa1; i <=0xff; i++)
  {
    lcd.setCursor(0, 0);
    lcd.print(String(i, HEX));
    lcd.print(" = ");
    lcd.print((char)i);
    delay(500);
  }
}
