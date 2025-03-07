#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>

#include "lcd.h"

#define MAX_ADMINS 5

int getFingerprintCount(Adafruit_Fingerprint &finger);

bool checkFingerStatus(Adafruit_Fingerprint &finger);

bool isEmptyID(Adafruit_Fingerprint &finger, uint16_t id);
bool isExistingID(Adafruit_Fingerprint &finger, uint16_t id);

bool fingerDetected(Adafruit_Fingerprint &finger);
bool fingerReleased(Adafruit_Fingerprint &finger);

bool isAdminLocation(uint16_t location);

/**
 * @note
 * User refers to non-admin fingerprint location.
 */
bool isUserLocation(uint16_t location);

int getAdminCount(Adafruit_Fingerprint &finger);

/**
 * @returns
 * Returns the location at which fingerprint is found.
 * Location is an integer from 1 to 127.
 * 0 is returned if fingerprint is not found.
 * Negative of the error code is returned when there is an error.
 */
int getFingerLocation(Adafruit_Fingerprint &finger);

/**
 * @note
 * Flashes some messages on LCD and detects a fingerprint.
 * If the fingerprint is admin, returns true else returns false.
 * It does not clear message or wait for user to see the LCD.
 * Use carefully.
 */
bool interactiveVerifyAdmin(Adafruit_Fingerprint &finger, LiquidCrystal_I2C &lcd);

void interactiveEnroll(Adafruit_Fingerprint &finger, LiquidCrystal_I2C &lcd, int newLocation);

void interactiveDelete(Adafruit_Fingerprint &finger, LiquidCrystal_I2C &lcd);
