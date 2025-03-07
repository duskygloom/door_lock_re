#include <cstdint>
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ROWS 4
#define LCD_COLS 20

#define FINGERPRINT_EMOJI 0
#define NO_FINGERPRINT_EMOJI 1
#define LOCKED_EMOJI 2
#define UNLOCKED_EMOJI 3
#define NO_ADMIN_EMOJI 4

extern uint8_t fingerprintEmoji[8];
extern uint8_t noFingerprintEmoji[8];
extern uint8_t lockedEmoji[8];
extern uint8_t unlockedEmoji[8];
extern uint8_t noAdminEmoji[8];

enum LineAlignment {
    ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT,
};

int getI2CAddress();
void printLine(LiquidCrystal_I2C &lcd, const String &lineText, int lineNumber, LineAlignment alignment);
void clearLine(LiquidCrystal_I2C &lcd, int lineNumber);
