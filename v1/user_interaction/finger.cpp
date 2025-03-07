#include <cstdint>
#include "finger.h"

#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>

#include "lcd.h"

int getFingerprintCount(Adafruit_Fingerprint &finger) {
    finger.getTemplateCount();
    return finger.templateCount;
}

bool checkFingerStatus(Adafruit_Fingerprint &finger) {
    bool isFingerActive = finger.verifyPassword();
    if (!isFingerActive)
        Serial.println("Fingerprint sensor not found.");
    return isFingerActive;
}

bool isEmptyID(Adafruit_Fingerprint &finger, uint16_t id) {
    return finger.loadModel(id) == FINGERPRINT_DBRANGEFAIL;
}

bool isExistingID(Adafruit_Fingerprint &finger, uint16_t id) {
    return finger.loadModel(id) == FINGERPRINT_OK;
}

bool fingerDetected(Adafruit_Fingerprint &finger) {
    return finger.getImage() == FINGERPRINT_OK;
}

bool fingerReleased(Adafruit_Fingerprint &finger) {
    return finger.getImage() == FINGERPRINT_NOFINGER;
}

bool isAdminLocation(uint16_t location) {
    return location >= 1 && location <= MAX_ADMINS;
}

/**
 * @note
 * User refers to non-admin fingerprint location.
 */
bool isUserLocation(uint16_t location) {
    return location > MAX_ADMINS && location <= 127;
}

int getAdminCount(Adafruit_Fingerprint &finger) {
    int numAdmins = 0;
    for (int i = 1; i <= MAX_ADMINS; ++i)
        if (isExistingID(finger, i)) ++numAdmins;
    return numAdmins;
}

/**
 * @returns
 * Returns the location at which fingerprint is found.
 * Location is an integer from 1 to 127.
 * 0 is returned if fingerprint is not found.
 * Negative of the error code is returned when there is an error.
 */
int getFingerLocation(Adafruit_Fingerprint &finger) {
    int code = finger.getImage(); // get image from sensor
    switch (code) {
        case FINGERPRINT_OK:
        // Serial.println("Detected a finger.");
        break;
        case FINGERPRINT_NOFINGER:
        // Serial.println("Did not detect a finger.");
        return -FINGERPRINT_NOFINGER;
        default:
        // Serial.println("Something went wrong when detecting finger.");
        return -code;
    }
    code = finger.image2Tz(); // create template from image
    switch (code) {
        case FINGERPRINT_OK:
        // Serial.println("Finger template created at slot 1.");
        break;
        case FINGERPRINT_IMAGEMESS:
        // Serial.println("Could not create template from image. Try again.");
        return -FINGERPRINT_IMAGEMESS;
        default:
        // Serial.println("Something went wrong when creating template.");
        return -code;
    }
    code = finger.fingerFastSearch(); // search fingerprint
    switch (code) {
        case FINGERPRINT_OK:
        // Serial.println(String("Found fingerprint at ") + finger.fingerID + " with a confidence of " + finger.confidence);
        return finger.fingerID;
        case FINGERPRINT_NOTFOUND:
        // Serial.println("Fingerprint not found.");
        return 0;
        default:
        // Serial.println("Something went wrong when searching fingerprint.");
        return -code;
    }
}

/**
 * @note
 * Flashes some messages on LCD and detects a fingerprint.
 * If the fingerprint is admin, returns true else returns false.
 * It does not clear message or wait for user to see the LCD.
 * Use carefully.
 */
bool interactiveVerifyAdmin(Adafruit_Fingerprint &finger, LiquidCrystal_I2C &lcd) {
    printLine(lcd, "Verify admin:", 3, ALIGN_CENTER);
    int location;
    // get a fingerprint
    while ((location = getFingerLocation(finger)) < 0) delay(100);
    if (!isAdminLocation(location)) { // if not admin, deny
        printLine(lcd, "Denied!", 3, ALIGN_CENTER);
        return false;
    }
    return true;
}

void interactiveEnroll(Adafruit_Fingerprint &finger, LiquidCrystal_I2C &lcd, int newLocation) {
    if (interactiveVerifyAdmin(finger, lcd)) { // admin detected
        printLine(lcd, "Release", 3, ALIGN_CENTER);
        // release admin finger
        while (!fingerReleased(finger)) delay(100);
        printLine(lcd, "New user:", 3, ALIGN_CENTER);
        // get next fingerprint
        int location;
        while ((location = getFingerLocation(finger)) < 0) delay(100);
        if (location != 0) { // if exists, show location
            printLine(lcd, String("Exists at ") + location, 3, ALIGN_CENTER);
        } else {
            printLine(lcd, "Release", 3, ALIGN_CENTER); // release for verify
            while (!fingerReleased(finger)) delay(100);
            printLine(lcd, "Again:", 3, ALIGN_CENTER); // verify
            while (!fingerDetected(finger)) delay(100);
            finger.image2Tz(2);
            if (finger.createModel() == FINGERPRINT_OK) { // model created
                if (finger.storeModel(newLocation) == FINGERPRINT_OK) { // successfully stored
                    printLine(lcd, String("Registered at ") + newLocation, 3, ALIGN_CENTER);
                } else { // failed
                    printLine(lcd, "Failed.", 3, ALIGN_CENTER);
                }
            } else {
                printLine(lcd, "Failed.", 3, ALIGN_CENTER);
            }
        }
    }
    delay(2000);
    clearLine(lcd, 3); // wait and clear
}

void interactiveDelete(Adafruit_Fingerprint &finger, LiquidCrystal_I2C &lcd) {
    if (interactiveVerifyAdmin(finger, lcd)) { // admin detected
        printLine(lcd, "Release", 3, ALIGN_CENTER);
        // release admin finger
        while (!fingerReleased(finger)) delay(100);
        printLine(lcd, "Delete user:", 3, ALIGN_CENTER);
        // get next fingerprint
        int location;
        while ((location = getFingerLocation(finger)) < 0) delay(100);
        if (location == 0) { // if does not exist, show message
            Serial.println("Fingerprint does not exist.");
            printLine(lcd, "Not found.", 3, ALIGN_CENTER);
        } else {
            printLine(lcd, "Release", 3, ALIGN_CENTER); // release
            while (!fingerReleased(finger)) delay(100);
            printLine(lcd, String("Detete ") + location + "?", 3, ALIGN_CENTER);
            delay(2000);
            if (interactiveVerifyAdmin(finger, lcd)) { // approved, delete
                int status = finger.deleteModel(location);
                if (status == FINGERPRINT_OK)
                    printLine(lcd, String("Deleted ") + location, 3, ALIGN_CENTER);
                else
                    printLine(lcd, "Failed.", 3, ALIGN_CENTER);

            }
        }
    }
    delay(2000);
    clearLine(lcd, 3); // wait and delete
}
