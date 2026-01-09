#ifndef CATALOG_H
#define CATALOG_H

#define MAX_COLUMNS 32
#define MAX_TEXT_LEN 255

typedef enum {
    COL_INT,
    COL_TEXT
} ColumnType;

typedef struct {
    char name[32];
    ColumnType type;
    int not_null;
    int unique;
    int primary_key;
} Column;

typedef struct {
    char name[32];
    Column columns[MAX_COLUMNS];
    int column_count;
} TableSchema;

int catalog_create_table(TableSchema *schema);
TableSchema *catalog_get_table(const char *name);

#endif
