#ifndef INDEX_H
#define INDEX_H

#include <stddef.h>

int index_create(const char *table, const char *column);
int index_insert(const char *table, const char *column, const char *key, long offset);
long index_lookup(const char *table, const char *column, const char *key);

#endif
