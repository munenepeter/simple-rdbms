#ifndef TABLE_H
#define TABLE_H

#include "catalog.h"
#include "row.h"

int table_insert(TableSchema *schema, Row *row);
int table_select(TableSchema *schema,
                 char **cols, int col_count,
                 const char *where_col,
                 const char *where_val);
int table_delete(TableSchema *schema,
                 const char *where_col,
                 const char *where_val);
int table_update(TableSchema *schema,
                 const char *where_col,
                 const char *where_val,
                 const char *set_col,
                 const char *set_val);

int table_join(TableSchema *s1, TableSchema *s2,
               const char *col1, const char *col2,
               const char *where_col, const char *where_val);
int table_select_at_offset(TableSchema *schema, char **cols, int col_count, long offset);

#endif