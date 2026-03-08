#include <Arduino.h>
#include "SdFunction/SdFunction.h"

SemaphoreHandle_t logMutex = NULL;

void setup() {
    Serial.begin(115200);
    delay(200);

    logMutex = xSemaphoreCreateMutex();
    if (logMutex == NULL) {
        Serial.println("Failed to create mutex");
        while (true) {
            delay(1000);
        }
    }
    
    sdReady = initSDCard();
}

void loop() {
    if (!sdReady) {
        delay(1000);
        return;
    }

    if (now - lastWriteTime >= WRITE_INTERVAL_MS) {
        lastWriteTime = now;
        LogWriteBuffer();
    }
}
