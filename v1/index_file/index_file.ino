/**
 * @note
 * THIS SKETCH IS USED TO UPLOAD THE HTML CODE
 * FOR THE WEB PAGE IN THE ESP32 FILE SYSTEM.
 */

#include <LittleFS.h>

#include "html_file.h"

void setup() {
    LittleFS.begin(true);
    File indexFile = LittleFS.open("/index.html", "w", true);
    indexFile.print(htmlText);
    indexFile.flush();
    indexFile.close();
    LittleFS.end();
}

void loop() {

}
