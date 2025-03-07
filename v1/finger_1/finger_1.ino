/**
 * @note
 * THIS SKETCH IS USED TO UPLOAD THE FIRST FINGERPRINT.
 * IT NEEDS NO AUTHENTICATION OR WHATSOEVER, JUST
 * SCANS A FINGER AND ADDS IT AT LOCATION 1 IN THE
 * FINGERPRINT SENSOR.
 */

#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>

#define LCD_COLS 20
#define LCD_ROWS 4

LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_Fingerprint finger(&Serial2);

enum LineAlignment {
    ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT,
};

void printLine(const String &lineText, int lineNumber, LineAlignment alignment) {
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

void clearLine(int lineNumber) {
    String content = "";
    for (int i = 0; i < LCD_COLS; ++i) content += " ";
    lcd.setCursor(0, lineNumber);
    lcd.print(content);
}

void setup() {
    // setup lcd
    lcd.init();
    lcd.clear();
    lcd.backlight();
    printLine("IoT Club", 0, ALIGN_CENTER);
    // setup finger
    finger.begin(57600);
    // start
    printLine("Finger 1:", 2, ALIGN_CENTER);
    while (finger.getImage() != FINGERPRINT_OK) delay(100);
    finger.image2Tz();
    printLine("Release", 2, ALIGN_CENTER);
    while (finger.getImage() != FINGERPRINT_NOFINGER) delay(100);
    printLine("Again:", 2, ALIGN_CENTER);
    while (finger.getImage() != FINGERPRINT_OK) delay(100);
    finger.image2Tz(2);
    int status = finger.createModel();
    if (status == FINGERPRINT_OK) {
        status = finger.storeModel(1);
        if (status != FINGERPRINT_OK)
            printLine("Failed.", 2, ALIGN_CENTER);
        else
            printLine("Success!", 2, ALIGN_CENTER);
    } else {
        printLine("Failed.", 2, ALIGN_CENTER);
    }
}

void loop() {

}
