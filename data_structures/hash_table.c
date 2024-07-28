#include <stdbool.h>
#include <sys/types.h>

typedef struct hash_table hash_table;

hash_table *hcreate(void);
void hdestroy(hash_table *table);
void *hget(hash_table *table, const char *key);
const char *hset(hash_table *table, const char *key, void *value);
size_t hsize(hash_table *table);

typedef struct
{
    const char *key;
    void *value;

    size_t _index;
    hash_table *_table;
} hash_table_iterator;

hash_table_iterator titerator(hash_table *table);
bool tnext(hash_table_iterator *iterator);