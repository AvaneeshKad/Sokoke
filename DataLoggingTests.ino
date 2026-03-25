#include "SPI.h"
#include "SD.h"

// Pin Definitions
const int chipSelect = 5;
const int ledPin = 2;
const int bootButton = 0; 

// Variables
bool isLogging = true;
unsigned long lastPress = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(bootButton, INPUT_PULLUP);

  // Initialize SPI. 1MHz is safe, but you can try 4000000 (4MHz) for more speed.
  SPI.begin(18, 19, 23, 5);
  
  if (!SD.begin(chipSelect, SPI, 1000000)) {
    Serial.println("SD initialization failed!");
    digitalWrite(ledPin, HIGH); 
    return;
  }

  // Create Header
  File file = SD.open("/mission.csv", FILE_WRITE);
  if (file) {
    file.println("UTC_Time,Latitude,Longitude,Alt_Baro,Pressure,Temp_C,Hum_pct,Accel_G,Current_mA");
    file.close();
    Serial.println("System Ready. Table Created.");
  }
}

void loop() {
  // 1. Check Button for Start/Stop
  if (digitalRead(bootButton) == LOW) {
    if (millis() - lastPress > 500) { 
      isLogging = !isLogging;
      lastPress = millis();
      
      if (!isLogging) {
        Serial.println("STOPPED: Safe to remove SD.");
        digitalWrite(ledPin, HIGH); 
      } else {
        Serial.println("RESUMED: Logging...");
        digitalWrite(ledPin, LOW);
      }
    }
  }

  // 2. Data Logging & Timing
  if (isLogging) {
    // Capture start time in microseconds
    unsigned long startWrite = micros();

    File file = SD.open("/mission.csv", FILE_APPEND);
    if (file) {
      // Data string
      String data = "120000,0.0000,0.0000,0.0,1013.25,25.0,50.0,1.0,0.0";
      
      file.println(data);
      
      // file.close() is where the physical "flush" to the SD card usually happens
      file.close(); 
      
      // Calculate elapsed time
      unsigned long duration = micros() - startWrite;

      // Output results
      Serial.print("Data: ");
      Serial.print(data);
      Serial.print(" | Write Time: ");
      Serial.print(duration);
      Serial.println(" us");

      // Success Blink
      digitalWrite(ledPin, HIGH);
      delay(50); 
      digitalWrite(ledPin, LOW);
    } else {
      Serial.println("Error opening file!");
    }

    delay(5000); // 5-second interval
  }
}