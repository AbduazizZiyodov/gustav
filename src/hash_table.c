#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "gc.h"
#include "hash_table.h"
#include "memory.h"
#include "object.h" // IWYU pragma: keep
#include "value.h"

void init_hash_table(HashTable *hash_table)
{
	hash_table->count = 0;
	hash_table->capacity = 0;
	hash_table->entries = NULL;
}

void free_tash_table(HashTable *hash_table)
{
	FREE_ARRAY(HashTableEntry, hash_table->entries, hash_table->capacity);
	init_hash_table(hash_table);
}

// TODO(abduaziz): nesting
static HashTableEntry *find_entry(HashTableEntry *entries, size_t capacity, StringObject *key)
{
	uint32_t index = key->hash & (capacity - 1); // capacity > 0

	HashTableEntry *tombstone = NULL;

	for (;;) {
		HashTableEntry *entry = &entries[index];

		if (entry->key == NULL) {
			if (Nil_Check(entry->value)) {
				return tombstone != NULL ? tombstone : entry;
			}
			if (tombstone == NULL) {
				tombstone = entry;
			}

		} else if (entry->key == key) {
			return entry;
		}

		index = (index + 1) & (capacity - 1);
	}
}

bool hash_table_get_item(HashTable *hash_table, StringObject *key, Value *value)
{
	if (hash_table->count == 0) {
		return false;
	}

	HashTableEntry *entry = find_entry(hash_table->entries, hash_table->capacity, key);

	if (entry->key is NULL) {
		return false;
	}
	*value = entry->value;
	return true;
}

static void adjust_capacity(HashTable *hash_table, size_t capacity)
{
	HashTableEntry *entries = ALLOCATE(HashTableEntry, capacity);

	for (size_t i = 0; i < capacity; i++) {
		entries[i].key = NULL;
		entries[i].value = NIL_VAL;
	}

	hash_table->count = 0;
	for (size_t i = 0; i < hash_table->capacity; i++) {
		HashTableEntry *entry = &hash_table->entries[i];

		if (entry->key is NULL) {
			continue;
		}

		HashTableEntry *dest = find_entry(entries, capacity, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;

		hash_table->count++;
	}

	FREE_ARRAY(HashTableEntry, hash_table->entries, hash_table->capacity);

	hash_table->entries = entries;
	hash_table->capacity = capacity;
}

bool hash_table_set_item(HashTable *hash_table, StringObject *key, Value value)
{
	if (hash_table->count + 1 >
	    (size_t)((double)hash_table->capacity * HASH_TABLE_MAX_LOAD_FACTOR)) {
		size_t capacity = GROW_CAPACITY(hash_table->capacity);
		adjust_capacity(hash_table, capacity);
	}

	HashTableEntry *entry = find_entry(hash_table->entries, hash_table->capacity, key);

	bool is_new_key = entry->key is NULL;

	if (is_new_key && Nil_Check(entry->value)) {
		hash_table->count++;
	}
	entry->key = key;
	entry->value = value;

	return is_new_key;
}

bool hash_table_delete_item(HashTable *hash_table, StringObject *key)
{
	if (hash_table->count == 0) {
		return false;
	}

	HashTableEntry *entry = find_entry(hash_table->entries, hash_table->capacity, key);

	if (entry->key is NULL) {
		return false;
	}

	entry->key = NULL;
	entry->value = BOOL_VAL(true);

	return true;
}

void hash_table_add_all(HashTable *from, HashTable *to)
{
	for (size_t i = 0; i < from->capacity; i++) {
		HashTableEntry *entry = &from->entries[i];

		if (entry->key != NULL) {
			hash_table_set_item(to, entry->key, entry->value);
		}
	}
}

StringObject *hash_table_find_string(HashTable *hash_table, const char *chars, size_t length,
				     uint32_t hash)
{
	if (hash_table->count == 0) {
		return NULL;
	}

	uint32_t index = hash & (hash_table->capacity - 1);

	while (true) {
		HashTableEntry *entry = &hash_table->entries[index];

		if (entry->key is NULL) {
			if (Nil_Check(entry->value)) {
				return NULL;
			}
		} else if (entry->key->length == length && entry->key->hash == hash &&
			   memcmp(entry->key->chars, chars, length) == 0) {
			return entry->key;
		}

		index = (index + 1) & (hash_table->capacity - 1);
	}
}

void hash_table_remove_white(HashTable *hash_table)
{
	for (size_t i = 0; i < hash_table->capacity; i++) {
		HashTableEntry *entry = &hash_table->entries[i];

		if (entry->key != NULL && !entry->key->obj.is_marked) {
			hash_table_delete_item(hash_table, entry->key);
		}
	}
}

void hash_table_mark(HashTable *table)
{
	for (size_t i = 0; i < table->capacity; i++) {
		HashTableEntry *entry = &table->entries[i];
		gc_mark_object((Object *)entry->key);
		gc_mark_value(entry->value);
	}
}
