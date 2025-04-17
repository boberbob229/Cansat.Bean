#include <SoftwareSerial.h>
#include <TinyGPS++.h>

SoftwareSerial gpsSerial(4, 3); // RX, TX
TinyGPSPlus gps;

unsigned long lastUpdate = 0;
const unsigned long interval = 2000;

void setup() {
    Serial.begin(9600);
    gpsSerial.begin(9600);
    Serial.println(F("📡 GPS Debug Monitor Starting..."));
    Serial.println(F("Waiting for GPS fix...\n"));
}

void loop() {
    // Read all incoming bytes from GPS
    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }

    // Update every 2 seconds
    if (millis() - lastUpdate >= interval) {
        lastUpdate = millis();
        Serial.println(F("========== GPS DEBUG =========="));

        // Fix status
        if (gps.location.isValid()) {
            Serial.print(F("📍 Location: "));
            Serial.print(gps.location.lat(), 6);
            Serial.print(F(", "));
            Serial.println(gps.location.lng(), 6);
        } else {
            Serial.println(F("📍 Location: Not fixed"));
        }

        // Satellites
        if (gps.satellites.isValid()) {
            Serial.print(F("🛰️  Satellites in view: "));
            Serial.println(gps.satellites.value());
        } else {
            Serial.println(F("🛰️  Satellites: Unknown"));
        }

        // Date
        if (gps.date.isValid()) {
            Serial.print(F("📅 Date: "));
            Serial.print(gps.date.year());
            Serial.print("-");
            Serial.print(gps.date.month());
            Serial.print("-");
            Serial.println(gps.date.day());
        } else {
            Serial.println(F("📅 Date: Invalid"));
        }

        // Time
        if (gps.time.isValid()) {
            Serial.print(F("⏰ Time (UTC): "));
            printDigits(gps.time.hour());
            Serial.print(":");
            printDigits(gps.time.minute());
            Serial.print(":");
            printDigits(gps.time.second());
            Serial.println();
        } else {
            Serial.println(F("⏰ Time: Invalid"));
        }

        Serial.println(F("================================"));
        Serial.println();
    }
}

// Helper: Print 2-digit numbers with leading 0
void printDigits(int digits) {
    if (digits < 10)
        Serial.print('0');
    Serial.print(digits);
}

