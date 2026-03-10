#pragma once

#include <Arduino.h>
#include <IridiumSBD.h>

constexpr float InvalidTemp = -512.0f;
constexpr float InvalidPressure = -1.0f;

typedef struct {
	uint64_t time;
	float temp;
	float pressure;
} __attribute__((packed)) TableEntry;

typedef struct {
	unsigned short size;
	unsigned short capacity;
	TableEntry *entries;
} Table;

typedef void * SerializedTable;

extern IridiumSBD IridiumModem;

Table *new_table();
void free_table(Table *t);
Table *checkTable(Table *t);
void add_entry(Table *t, TableEntry e);
Table *add_sensor_data(Table *t, uint64_t time, float temp, float pressure);
void seal_table(Table *t);
SerializedTable serialize_table(Table *t);
Table *deserialize_table(SerializedTable t);
size_t table_memsize(Table *t);
void send_table(Table *t);
