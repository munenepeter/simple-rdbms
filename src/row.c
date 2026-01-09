#include <stdlib.h>
#include <string.h>
#include "row.h"
#include "util.h"

// Calculate fixed row size based on schema
size_t row_size(TableSchema *schema) {
    size_t sz = 0;
    for (int i = 0; i < schema->column_count; i++) {
        if (schema->columns[i].type == COL_INT) {
            sz += sizeof(int);
        } else {
            sz += MAX_TEXT_LEN + 1;
        }
    }
    return sz;
}

Row *row_create(TableSchema *schema, char **values) {
    size_t rs = row_size(schema);
    Row *row = xmalloc(sizeof(Row));
    row->data = xmalloc(rs);
    memset(row->data, 0, rs);

    char *ptr = (char *)row->data;
    for (int i = 0; i < schema->column_count; i++) {
        if (schema->columns[i].type == COL_INT) {
            int val = atoi(values[i]);
            memcpy(ptr, &val, sizeof(int));
            ptr += sizeof(int);
        } else {
            strncpy(ptr, values[i], MAX_TEXT_LEN);
            ptr[MAX_TEXT_LEN] = '\0';
            ptr += MAX_TEXT_LEN + 1;
        }
    }

    return row;
}

void row_free(Row *row) {
    if (!row) return;
    free(row->data);
    free(row);
}
