#pragma once

#include <stdint.h>

#include "value.h"

typedef struct {
	StringObject *key;
	Value value;
} HashTableEntry;

typedef struct {
	size_t count;
	size_t capacity;
	HashTableEntry *entries;
} HashTable;

void HashTable_Init(HashTable *hash_table);
void HashTable_Free(HashTable *hash_table);

bool HashTable_SetItem(HashTable *hash_table, StringObject *key, Value value);
bool HashTable_GetItem(HashTable *hash_table, StringObject *key, Value *value);
bool HashTable_DelItem(HashTable *hash_table, StringObject *key);
void HashTable_AddAll(HashTable *from, HashTable *to);
void HashTable_RemoveWhite(HashTable *hash_table);

StringObject *HashTable_FindString(HashTable *hash_table, const char *chars,
				   size_t length, uint32_t hash);

void HashTable_Mark(HashTable *table);
