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

void init_hash_table(HashTable *hash_table);
void free_tash_table(HashTable *hash_table);

bool hash_table_set_item(HashTable *hash_table, StringObject *key, Value value);
bool hash_table_get_item(HashTable *hash_table, StringObject *key, Value *value);
bool hash_table_delete_item(HashTable *hash_table, StringObject *key);
void hash_table_add_all(HashTable *from, HashTable *to);
void hash_table_remove_white(HashTable *hash_table);

StringObject *hash_table_find_string(HashTable *hash_table, const char *chars, size_t length,
				     uint32_t hash);

void hash_table_mark(HashTable *table);
