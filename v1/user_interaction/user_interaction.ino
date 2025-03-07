/**
 * @note
 * THIS SKETCH HANDLES THE VARIOUS AUTHENTICATION
 * METHODS, HOSTS THE WEB PAGE AND API FOR THE 
 * DOOR LOCK UNIT.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiMulti.h>
#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <IRremote.hpp>
#include <LittleFS.h>
#include <SPI.h>
#include <MFRC522.h>

#include "secret.h"
#include "ir.h"
#include "lcd.h"
#include "finger.h"


/* DOOR */

#define DOOR_OPEN_DURATION 5000
unsigned long lastDoorOpen = 0;

static inline bool isDoorOpen() {
    return lastDoorOpen == 0 ? false : millis() - lastDoorOpen <= DOOR_OPEN_DURATION;
}

String openReason = "";


/* BUTTON */

#define BUTTON_PIN 39

static inline bool isButtonOn() {
    return digitalRead(BUTTON_PIN) == LOW;
}


/* SERIAL */

void setupSerial() {
    Serial.begin(115200);
    Serial.println("\n\n---------- New session ----------\n");
}


/* WIFI */

WiFiMulti wifiMulti;

void setupWifi() {
    WiFi.mode(WIFI_STA);
    for (int i = 0; i < wifiCount; ++i) {
        wifiMulti.addAP(ssid[i].c_str(), password[i].c_str());
    }
}


/* MDNS */

#define HOSTNAME "doorlock"
#define SERVER_PORT 80

void setupMDNS() {
    if (MDNS.begin(HOSTNAME)) {
        Serial.println(String("Hostname: ") + HOSTNAME);
        if (MDNS.addService("http", "tcp", SERVER_PORT))
            Serial.println(String("Service started at port ") + SERVER_PORT);
    }
}


/* FINGERPRINT */

Adafruit_Fingerprint finger(&Serial2);
bool isFingerActive = false;
int fingerCount = 0;
int adminCount = 0;
int lastFinger = 0;

void setupFingerprint() {
    finger.begin(57600);
    isFingerActive = checkFingerStatus(finger);
    adminCount = getAdminCount(finger);
    fingerCount = getFingerprintCount(finger);
}

/**
 * @todo
 * Function to add new admin and non-admin fingers.
 * Function to delete fingers.
 */


/* IR REMOTE */

#define IR_PIN 36

IrKey lastIR = KOther;

IrKey getKey(IRData data) {
    switch (data.decodedRawData >> 16) { // last 4 hex are bf00 for all commands
        case 0xff00: return K1;
        case 0xfe01: return K2;
        case 0xfd02: return K3;
        case 0xfb04: return K4;
        case 0xfa05: return K5;
        case 0xf906: return K6;
        case 0xf708: return K7;
        case 0xf609: return K8;
        case 0xf50a: return K9;
        case 0xf30c: return KStar;
        case 0xf20d: return K0;
        case 0xf10e: return KHash;
        case 0xee11: return KUp;
        case 0xeb14: return KLeft;
        case 0xea15: return KOk;
        case 0xe916: return KRight;
        case 0xe619: return KDown;
        default: return KOther;
    }
}

void setupIR() {
    IrReceiver.begin(IR_PIN, true);
}


/* RFID */

/**
 * @note
 * Discontinued for now.
 * Will be included in the next version.
 */

#define RST_PIN 34

MFRC522 rfid(RST_PIN);

void setupRFID() {
    SPI.begin();
    rfid.PCD_Init();
    rfid.PCD_DumpVersionToSerial();
}


/* LCD */

LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS);

void setupLCD() {
    lcd.init();
    lcd.clear();
    lcd.backlight();
    lcd.createChar(FINGERPRINT_EMOJI, fingerprintEmoji);
    lcd.createChar(NO_FINGERPRINT_EMOJI, noFingerprintEmoji);
    lcd.createChar(LOCKED_EMOJI, lockedEmoji);
    lcd.createChar(UNLOCKED_EMOJI, unlockedEmoji);
    lcd.createChar(NO_ADMIN_EMOJI, noAdminEmoji);
    printLine(lcd, "IoT Club", 0, ALIGN_CENTER);
    Serial.println("LCD setup complete.");
}


/* WEBSERVER */

WebServer server(SERVER_PORT);

void setupServer() {
    server.on("/", []() {
        LittleFS.begin();
        File indexFile = LittleFS.open("/index.html");
        server.send(200, "text/html", indexFile.readString());
        indexFile.close();
        LittleFS.end();
    });

    server.on("/status", []() {
        JsonDocument doc;
        doc["finger-status"] = isFingerActive;
        doc["admin-count"] = adminCount;
        doc["finger-count"] = fingerCount;
        doc["last-fingerprint"] = lastFinger;
        doc["last-ir-key"] = getKeyString(lastIR);
        doc["door-open-second"] = lastDoorOpen / 1000;
        doc["door-open-reason"] = openReason.isEmpty() ? "-" : openReason;
        doc["door-status"] = isDoorOpen() ? "Open" : "Closed";
        char buffer[1000];
        serializeJson(doc, buffer);
        server.send(200, "application/json", buffer);
    });

    server.on("/doorstatus", []() {
        JsonDocument doc;
        doc["door-status"] = millis() - lastDoorOpen > DOOR_OPEN_DURATION ? "Closed" : "Open";
        char buffer[1000];
        serializeJson(doc, buffer);
        server.send(200, "application/json", buffer);
    });

    server.on("/opendoor", []() {
        if (!server.authenticate(USERNAME, PASSWORD)) {
            server.requestAuthentication(DIGEST_AUTH, "Login required", "Authentication failed.");
            server.send(301);
        } else {
            lastDoorOpen = millis();
            openReason = "Request";
            server.send(200);
        }
    });

    server.onNotFound([]() {
        server.send(404, "plain/text", "You entered an uncharted area.");
    });

    server.begin();
    Serial.println("Web server started!");
}


/* TASKS */

TaskHandle_t serverTaskHandle;
TaskHandle_t buttonTaskHandle;
TaskHandle_t rifdTaskHandle;
TaskHandle_t sensorTaskHandle;
TaskHandle_t displayTaskHandle;

void serverTask(void *params) {
    while (true) {
        if (wifiMulti.run() == WL_CONNECTED)
            server.handleClient();
        else
            Serial.println("Failed to connect to Wi-Fi.");
        delay(100);
    }
}

/**
 * @note
 * Button and request work independently, so they can be used
 * even when new users are being registered.
 */
void buttonTask(void *params) {
    while (true) {
        if (isButtonOn()) {
            lastDoorOpen = millis();
            openReason = "Button";
        }
        delay(100);
    }
}

void rfidTask(void *params) {

}

void sensorTask(void *params) {
    while (true) {
        // sense fingerprint
        isFingerActive = checkFingerStatus(finger);
        fingerCount = getFingerprintCount(finger);
        adminCount = getAdminCount(finger);
        int location = getFingerLocation(finger);
        if (location < 0) {
            switch (-location) {
                case FINGERPRINT_NOFINGER:
                Serial.println("Did not detect a finger.");
                break;
                default:
                Serial.print("Something went wrong when trying to get fingerprint location: 0x");
                Serial.println(String(-location < 0x10 ? "0" : "") + String(-location, HEX));
            }
        } else if (location == 0) {
            Serial.println("Fingerprint is not registered.");
        } else if (location <= MAX_ADMINS) {
            Serial.println("Detected an admin fingerprint.");
            lastFinger = location;
            lastDoorOpen = millis();
            openReason = "Admin";
        } else {
            Serial.println("Detected a registered fingerprint.");
            lastFinger = location;
            lastDoorOpen = millis();
            openReason = "Finger";
        }
        // sense IR
        if (IrReceiver.decode()) {
            lastIR = getKey(IrReceiver.decodedIRData);
            switch (lastIR) {
                case KOk:
                lastDoorOpen = millis();
                openReason = "Remote";
                break;
                case K0:
                int newLocation;
                for (newLocation = 1; newLocation <= MAX_ADMINS; ++newLocation)
                    if (finger.loadModel(newLocation) == FINGERPRINT_DBRANGEFAIL) { // first empty admin slot
                        printLine(lcd, "Enroll admin", 1, ALIGN_CENTER);
                        interactiveEnroll(finger, lcd, newLocation);
                        clearLine(lcd, 1);
                        break;
                    }
                if (newLocation > MAX_ADMINS) {
                    printLine(lcd, "Too many admins.", 3, ALIGN_CENTER);
                }
                break;
                case K1:
                for (newLocation = MAX_ADMINS + 1; newLocation <= 127; ++newLocation)
                    if (finger.loadModel(newLocation) == FINGERPRINT_DBRANGEFAIL) { // first empty user slot
                        printLine(lcd, "Enroll user", 1, ALIGN_CENTER);
                        interactiveEnroll(finger, lcd, newLocation);
                        clearLine(lcd, 1);
                        break;
                    }
                if (newLocation > 127) {
                    printLine(lcd, "Too many users.", 3, ALIGN_CENTER);
                }
                break;
                case K2:
                printLine(lcd, "Delete", 1, ALIGN_CENTER);
                interactiveDelete(finger, lcd);
                clearLine(lcd, 1);
                break;
            }
            Serial.println(String("IR remote: ") + getKeyString(lastIR));
            IrReceiver.resume();
        }
        delay(500);
    }
}

void displayTask(void *params) {
    while (true) {
        // door icon
        if (isDoorOpen()) {
            lcd.setCursor(0, 0);
            lcd.write(UNLOCKED_EMOJI);
            printLine(lcd, String("Reason: ") + openReason, 2, ALIGN_CENTER);
        } else {
            lcd.setCursor(0, 0);
            lcd.write(LOCKED_EMOJI);
            clearLine(lcd, 2);
        }
        // admin count icon
        if (adminCount == 0) {
            lcd.setCursor(LCD_COLS-1, 0);
            lcd.write(NO_ADMIN_EMOJI);
        } else {
            lcd.setCursor(LCD_COLS-1, 0);
            lcd.write(' ');
        }
        delay(500);
    }
}


/* BOARD SETUP */

void setup() {
    setupSerial();
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    setupWifi();
    setupMDNS();
    setupFingerprint();
    setupLCD();
    setupIR();
    // setupRFID();
    setupServer();
    // tasks
    xTaskCreate(serverTask, "serverTask", 10000, NULL, 0, &serverTaskHandle);
    xTaskCreate(buttonTask, "buttonTask", 10000, NULL, 0, &buttonTaskHandle);
    xTaskCreate(sensorTask, "sensorTask", 10000, NULL, 0, &sensorTaskHandle);
    xTaskCreate(displayTask, "displayTask", 10000, NULL, 0, &displayTaskHandle);
}

void loop() {
    
}
