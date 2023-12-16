// Program Uji LCD16x2
  // melakukan pengujian komponen LCD16x2
// KAMUS
  // Variabel
    //
// ALGORITMA UTAMA
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//LCD_I2C lcd(0x27, 16, 2); // Default address of most PCF8574 modules,
// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

void setup()
{
  Serial.begin(115200);

  lcd.begin();  // If you are using more I2C devices using the Wire library use lcd.begin(false)
                // this stop the library(LCD_I2C) from calling Wire.begin()
  lcd.backlight();
}

void loop()
{
  lcd.setCursor(0, 0);
  lcd.print("Pengujian LCD16x2");
  lcd.setCursor(0, 1);
  lcd.print("Berhasil");
}
