#include <cstdint>
#include "lcd.h"

uint8_t fingerprintEmoji[8] = {
    B01110, B10011, B10111, B11111,
    B11101, B11001, B01110, B00000,
};

uint8_t noFingerprintEmoji[8] = {
    B01110, B11001, B10001, B10001,
    B10001, B10011, B01110, B00000,
};

uint8_t lockedEmoji[8] = {
    B11111, B10001, B10001, B11111,
    B11111, B11111, B11111, B00000,
};

uint8_t unlockedEmoji[8] = {
    B11111, B00001, B00001, B11111,
    B11111, B11111, B11111, B00000,
};

uint8_t noAdminEmoji[8] = {
    B00000, B00001, B11101, B10101,
    B10101, B11100, B10101, B00000,
};

int getI2CAddress() {
    uint8_t address = -1;
    Wire.begin();
    for (uint8_t i = 0; i < 127; ++i) {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0) {
            address = i;
            break;
        }
    }
    Wire.end();
    return address >= 0 && address < 127 ? address : -1;
}

void printLine(LiquidCrystal_I2C &lcd, const String &lineText, int lineNumber, LineAlignment alignment) {
    String subText = lineText.substring(0, LCD_COLS);
    String content = "";
    // add leading spaces
    int leadingSpaces = 0;
    switch (alignment) {
        case ALIGN_LEFT:
        break;
        case ALIGN_CENTER:
        leadingSpaces = (LCD_COLS - subText.length()) / 2;
        break;
        case ALIGN_RIGHT:
        leadingSpaces = LCD_COLS - subText.length();
        break;
    }
    for (int i = 0; i < leadingSpaces; ++i) content += " ";
    content += subText;
    int trailingSpaces = LCD_COLS - leadingSpaces - subText.length();
    for (int i = 0; i < trailingSpaces; ++i) content += " ";
    lcd.setCursor(0, lineNumber % LCD_ROWS);
    lcd.print(content);
}

void clearLine(LiquidCrystal_I2C &lcd, int lineNumber) {
    String content = "";
    for (int i = 0; i < LCD_COLS; ++i) content += " ";
    lcd.setCursor(0, lineNumber);
    lcd.print(content);
}
