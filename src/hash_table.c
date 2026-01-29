#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "hash_table.h"
#include "memory.h"
/* NOLINTNEXTLINE(misc-include-cleaner) */
#include "object.h"
#include "value.h"

void init_hash_table(hash_table_t *hash_table)
{
	hash_table->count = 0;
	hash_table->capacity = 0;
	hash_table->entries = NULL;
}

void free_hash_table(hash_table_t *hash_table)
{
	FREE_ARRAY(ht_entry_t, hash_table->entries, hash_table->capacity);
	init_hash_table(hash_table);
}

// TODO(abduaziz): nesting
static ht_entry_t *find_entry(ht_entry_t *entries, size_t capacity,
			      string_t *key)
{
	/* NOLINTNEXTLINE(clang-analyzer-core.DivideZero) */
	uint32_t index = key->hash % capacity; // capacity > 0

	ht_entry_t *tombstone = NULL;

	for (;;) {
		ht_entry_t *entry = &entries[index];

		if (entry->key == NULL) {
			if (IS_NIL(entry->value)) {
				return tombstone != NULL ? tombstone : entry;
			}
			if (tombstone == NULL) {
				tombstone = entry;
			}

		} else if (entry->key == key) {
			return entry;
		}

		index = (index + 1) % capacity;
	}
}

bool ht_get(hash_table_t *hash_table, string_t *key, value_t *value)
{
	if (hash_table->count == 0) {
		return false;
	}

	ht_entry_t *entry =
		find_entry(hash_table->entries, hash_table->capacity, key);

	if (entry->key is NULL) {
		return false;
	}
	*value = entry->value;
	return true;
}

static void adjust_capacity(hash_table_t *hash_table, size_t capacity)
{
	ht_entry_t *entries = ALLOCATE(ht_entry_t, capacity);

	for (size_t i = 0; i < capacity; i++) {
		entries[i].key = NULL;
		entries[i].value = NIL_VAL;
	}

	hash_table->count = 0;
	for (size_t i = 0; i < hash_table->capacity; i++) {
		ht_entry_t *entry = &hash_table->entries[i];

		if (entry->key is NULL) {
			continue;
		}

		ht_entry_t *dest = find_entry(entries, capacity, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;

		hash_table->count++;
	}

	FREE_ARRAY(ht_entry_t, hash_table->entries, hash_table->capacity);

	hash_table->entries = entries;
	hash_table->capacity = capacity;
}

bool ht_insert(hash_table_t *hash_table, string_t *key, value_t value)
{
	if (hash_table->count + 1 > (size_t)((double)hash_table->capacity *
					     HASH_TABLE_MAX_LOAD_FACTOR)) {
		size_t capacity = GROW_CAPACITY(hash_table->capacity);
		adjust_capacity(hash_table, capacity);
	}

	ht_entry_t *entry =
		find_entry(hash_table->entries, hash_table->capacity, key);

	bool is_new_key = entry->key is NULL;

	if (is_new_key && IS_NIL(entry->value)) {
		hash_table->count++;
	}
	entry->key = key;
	entry->value = value;

	return is_new_key;
}

bool ht_delete(hash_table_t *hash_table, string_t *key)
{
	if (hash_table->count == 0) {
		return false;
	}

	ht_entry_t *entry =
		find_entry(hash_table->entries, hash_table->capacity, key);

	if (entry->key is NULL) {
		return false;
	}

	entry->key = NULL;
	entry->value = BOOL_VAL(true);

	return true;
}

void ht_add_all(hash_table_t *from, hash_table_t *to)
{
	for (size_t i = 0; i < from->capacity; i++) {
		ht_entry_t *entry = &from->entries[i];

		if (entry->key != NULL) {
			ht_insert(to, entry->key, entry->value);
		}
	}
}

string_t *ht_find_string(hash_table_t *hash_table, const char *chars,
			 size_t length, uint32_t hash)
{
	if (hash_table->count == 0) {
		return NULL;
	}

	uint32_t index = hash % hash_table->capacity;

	while (true) {
		ht_entry_t *entry = &hash_table->entries[index];

		if (entry->key is NULL) {
			if (IS_NIL(entry->value)) {
				return NULL;
			}
		} else if (entry->key->length == length &&
			   entry->key->hash == hash &&
			   memcmp(entry->key->chars, chars, length) == 0) {
			return entry->key;
		}

		index = (index + 1) % hash_table->capacity;
	}
}
