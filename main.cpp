#include <Arduino.h>
#include "SdFunction/SdFunction.h"
#include "RockblockFunction/RockblockFunction.h"
#include <freertos/FreeRTOS.h>

SemaphoreHandle_t logMutex = NULL;

void readCore();
void writeCore();

constexpr BaseType_t SENSOR_CORE_ID = 0;
constexpr BaseType_t WRITE_CORE_ID = 1;

void sensorTask(void*) {
    readCore();
}

void sdWriteTask(void*) {
    writeCore();
}


void readCore() {
    while (true) {
        // randomSensorData(); // Test data generation
        delay(10);
    }
}

void writeCore() {
    while (true) {
        uint32_t now = millis();
        if (now - lastWriteTime >= WRITE_INTERVAL_MS) {
            if (!LogWriteBuffer()) {
                Serial.println("Failed to write log buffer to SD");
            } else {
                if (!sendRockblockBuffer()) {
                    Serial.println("Failed to send rockblock buffer");
                }
                lastWriteTime = now;
            }
        }
        delay(1000);
    }
}

void setup() {
    Serial.begin(115200);
    delay(200);
    randomSeed((uint32_t)esp_random());

    logMutex = xSemaphoreCreateMutex(); // Create mutex 
    if (logMutex == NULL) {
        Serial.println("Failed to create mutex");
        while (true) {
            delay(1000);
        }
    }
    
    sdReady = initSDCard();
    if (!sdReady) {
        delay(1000);
        return;
    }

    if (!initRockblockBuffer()) {
        Serial.println("Failed to initialize rockblock buffer");
    }

    xTaskCreatePinnedToCore(
        sensorTask,
        "SensorDataTask",
        4096,
        NULL,
        1,
        NULL,
        SENSOR_CORE_ID
    );
    
    xTaskCreatePinnedToCore(
        sdWriteTask,
        "SDWriteTask",
        4096,
        NULL,
        1,
        NULL,
        WRITE_CORE_ID
    );
}

void loop() {}
