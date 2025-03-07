/**
 * @note
 * THIS IS THE SKETCH FOR THE ACTUAL DOOR LOCK WITH
 * THE SOLENOID. IT COMMUNICATES WITH THE USER
 * INTERACTION SKETCH USING AN API.
 */

#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#else
#error "Program only supports ESP32 and ESP8266. Things will definitely go wrong!"
#endif

#include <ArduinoJson.h>
#include <WiFiClient.h>

#include "secret.h"

#define HOSTNAME "doorlock"
#define SERVER_PORT 80
#define WIFI_CONNECTION_TIMEOUT 20000

#if defined(ESP8266)
ESP8266WiFiMulti wifiMulti;
#elif defined(ESP32)
WiFiMulti wifiMulti;
#endif

#define RELAY_PIN 14

void setupSerial() {
#if defined(ESP8266)
    Serial.begin(9600);
#elif defined(ESP32)
    Serial.begin(115200);
#endif
    Serial.println("\n\n---------- New session ----------\n");
}

void setupWifi() {
    WiFi.mode(WIFI_STA);
    for (int i = 0; i < wifiCount; ++i)
        wifiMulti.addAP(ssid[i].c_str(), password[i].c_str());
}

void setup() {
    setupSerial();
    setupWifi();
    pinMode(RELAY_PIN, OUTPUT);
}

bool isDoorOpen() {
    if (wifiMulti.run(2000) == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        String url = String("http://" HOSTNAME ".local:") + String(SERVER_PORT) + "/doorstatus";
        Serial.println(String("HTTP connection with ") + url);
        if (http.begin(client, url)) {
            int status = http.GET();
            if (status == 200) {
                String body = http.getString();
                JsonDocument doc;
                deserializeJson(doc, body);
                if (doc["door-status"] == "Open") {
                    Serial.println("Open");
                    return true;
                }
            } else {
                Serial.println(String("Invalid response code: ") + String(status));
            }
            http.end();
        } else {
            Serial.println("Failed to establish HTTP connection.");
        }
    } else {
        Serial.println("Failed to connect to Wi-Fi.");
    }
    Serial.println("Closed");
    return false;
}

void loop() {
    if (isDoorOpen())
        digitalWrite(RELAY_PIN, LOW);
    else
        digitalWrite(RELAY_PIN, HIGH);
    delay(500);
}
