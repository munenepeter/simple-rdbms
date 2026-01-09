#ifndef ROW_H
#define ROW_H

#include "catalog.h"

typedef struct {
    void *data;
} Row;

Row *row_create(TableSchema *schema, char **values);
void row_free(Row *row);
size_t row_size(TableSchema *schema);

#endif
