#ifndef GUSTAV_HASHTABLE_H
#define GUSTAV_HASHTABLE_H

#include <stdint.h>

#include "value.h"

typedef struct {
	string_t *key;
	value_t value;
} ht_entry_t;

typedef struct {
	size_t count;
	size_t capacity;
	ht_entry_t *entries;
} hash_table_t;

void init_hash_table(hash_table_t *hash_table);
void free_hash_table(hash_table_t *hash_table);

bool ht_insert(hash_table_t *hash_table, string_t *key, value_t value);
bool ht_get(hash_table_t *hash_table, string_t *key, value_t *value);
bool ht_delete(hash_table_t *hash_table, string_t *key);
void ht_add_all(hash_table_t *from, hash_table_t *to);

string_t *ht_find_string(hash_table_t *hash_table, const char *chars,
			 size_t length, uint32_t hash);

void mark_table(hash_table_t *table);

#endif // GUSTAV_HASHTABLE_H
