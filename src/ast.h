#ifndef AST_H
#define AST_H

#include "catalog.h"

//for now just create table, insert, and select
typedef enum {
    AST_CREATE_TABLE,
    AST_INSERT,
    AST_SELECT,
    AST_UPDATE,
    AST_DELETE,
    AST_CREATE_DATABASE,
    AST_DESCRIBE_TABLE
} AstType;

typedef struct {
    char *name;
    int   type;
    int   not_null;
    int   unique;
    int   primary_key;
} ColumnDef;

typedef struct {
    char *db_name;
} CreateDatabaseStmt;

typedef struct {
    char *table_name;
} DescribeTableStmt;

typedef struct {
    char *table_name;
    ColumnDef *columns;
    int column_count;
} CreateTableStmt;

typedef struct {
    char *table_name;
    char **values;
    int value_count;
} InsertStmt;

typedef struct {
    char *table_name;
    char **columns;
    int column_count;
    char *where_column;
    char *where_value;
    
    // Join support
    char *join_table;
    char *join_left_col;
    char *join_right_col;
} SelectStmt;

typedef struct {
    char *table_name;
    char *set_column;
    char *set_value;
    char *where_column;
    char *where_value;
} UpdateStmt;

typedef struct {
    char *table_name;
    char *where_column;
    char *where_value;
} DeleteStmt;

typedef struct {
    AstType type;
    union {
        CreateTableStmt    create_table;
        CreateDatabaseStmt create_database;
        DescribeTableStmt  describe_table;
        InsertStmt         insert;
        SelectStmt         select;
        UpdateStmt         update;
        DeleteStmt         delete_stmt;
    };
} AstNode;

#endif
