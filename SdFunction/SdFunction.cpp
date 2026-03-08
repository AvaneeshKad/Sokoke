#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "SdFunction.h"

#define SD_CS_PIN 5
#define MUTEX_TIMEOUT_MS 5000

const uint32_t WRITE_INTERVAL_MS = 1000;
const char* CSV_FILE_PATH = "/sensor_log.csv";

uint32_t lastWriteTime = 0;
bool sdReady = false;

char logBuffer[512];
size_t logBufferlen = 0;

static const char* const CSV_COLUMNS[] = {"temp", "pressure"};
static const int NUM_COLUMNS = sizeof(CSV_COLUMNS) / sizeof(CSV_COLUMNS[0]);

bool initSDCard() {
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Card Mount Failed");
        return false;
    }

    uint64_t cardSize = SD.cardSize() / (1000 * 1000 * 1000);
    Serial.printf("SD Card Size: %lluGB\n", cardSize);

    if (!SD.exists(CSV_FILE_PATH)) {
        File headerFile = SD.open(CSV_FILE_PATH, FILE_WRITE);
        if (!headerFile) {
            Serial.println("Failed to create CSV file");
            return false;
        }
        headerFile.print("timestamp_ms");
        for (int i = 0; i < NUM_COLUMNS; i++) {
            headerFile.print(",");
            headerFile.print(CSV_COLUMNS[i]);
        }
        headerFile.println();
        headerFile.close();
    }

    return true;
}

bool LogWriteBuffer() { 
    if (logBufferlen == 0) {
        return true;
    }
    
    if (xSemaphoreTake(logMutex, MUTEX_TIMEOUT_MS) != pdTRUE) {
        Serial.println("Failed to lock log mutex");
        return false;
    }

    File logFile = SD.open(CSV_FILE_PATH, FILE_APPEND);
    if (!logFile) {
        Serial.println("Failed to open CSV file");
        xSemaphoreGive(logMutex);
        return false;
    }

    size_t bytesWritten = logFile.write((const uint8_t*)logBuffer, logBufferlen);
    logFile.close();

    xSemaphoreGive(logMutex);

        if (bytesWritten != logBufferlen) {
        Serial.println("partial write to CSV file");
        return false;
    }

    logBufferlen = 0;
    return true;
}


void writeDataToBuffer(const char* name, float value) { 
    for (int i = 0; i < NUM_COLUMNS; i++) {
        if (strcmp(name, CSV_COLUMNS[i]) == 0) {

            char entry[64];

            int pos = snprintf(entry, sizeof(entry), "%u", millis());

            for (int j = 0; j < NUM_COLUMNS; j++) {
                if (j == i) {
                    int written = snprintf(entry + pos, sizeof(entry) - pos, ",%.2f", value);
                    if (written < 0 || pos + written >= (int)sizeof(entry)) {
                        Serial.println("Failed to format CSV row");
                        return;
                    }
                    pos += written;
                } else {
                    entry[pos++] = ',';
                }
            }
            entry[pos++] = '\n';

            if (xSemaphoreTake(logMutex, MUTEX_TIMEOUT_MS) != pdTRUE) {
                Serial.println("Failed to lock log mutex");
                return;
            }

            if (logBufferlen + pos >= sizeof(logBuffer)) {
                Serial.println("Log buffer overflow, flushing to SD");
                xSemaphoreGive(logMutex);
                if (!LogWriteBuffer()) {
                    return;
                }
                if (xSemaphoreTake(logMutex, MUTEX_TIMEOUT_MS) != pdTRUE) {
                    Serial.println("Failed to re-lock log mutex");
                    return;
                }
            }

            memcpy(logBuffer + logBufferlen, entry, pos);
            logBufferlen += pos;

            xSemaphoreGive(logMutex);
            return;
        }
    }
}
