#pragma once

#include <Arduino.h>
#include "SdFunction/SdFunction.h"
#include "RockblockFunction/RockblockFunction.h"
#include <freertos/FreeRTOS.h>

constexpr float InvalidTemperature = -512.0f;
constexpr float InvalidHumidity = -1.0f;
constexpr float InvalidPressure = -1.0f;

typedef enum {
	ATH30_Temperature,
	ATH30_Humidity,
	BMP390_Temperature,
	BMP390_Pressure,

	SENSOR_COUNT,
} SensorDataType;

inline const char *get_sensor_name(SensorDataType type) {
    static const char *names[] = {
        "ATH30_temperature",
        "ATH30_humidity",
        "BMP390_temperature",
        "BMP390_pressure",
    };
    return names[type];
}
constexpr float const INVALID_RESPONSES[] = {InvalidTemperature, InvalidPressure, InvalidTemperature, InvalidHumidity};

