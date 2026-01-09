#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "sql.h"
#include "lexer.h"
#include "parser.h"
#include "catalog.h"
#include "table.h"
#include "row.h"
#include "errors.h"
#include "storage.h"
#include "index.h"

const char *token_type_to_string(TokenType type) {
    switch (type) {
        case TOK_EOF: return "EOF";
        case TOK_IDENTIFIER: return "IDENTIFIER";
        case TOK_NUMBER: return "NUMBER";
        case TOK_STRING: return "STRING";
        case TOK_COMMA: return "COMMA";
        case TOK_SEMICOLON: return "SEMICOLON";
        case TOK_LPAREN: return "LPAREN";
        case TOK_RPAREN: return "RPAREN";
        case TOK_EQUAL: return "EQUAL";
        case TOK_STAR: return "STAR";
        case TOK_SELECT: return "SELECT";
        case TOK_INSERT: return "INSERT";
        case TOK_CREATE: return "CREATE";
        case TOK_TABLE: return "TABLE";
        case TOK_INTO: return "INTO";
        case TOK_VALUES: return "VALUES";
        case TOK_WHERE: return "WHERE";
        case TOK_INT: return "INT";
        case TOK_TEXT: return "TEXT";
        case TOK_PRIMARY: return "PRIMARY";
        case TOK_KEY: return "KEY";
        case TOK_UNIQUE: return "UNIQUE";
        case TOK_NOT: return "NOT";
        case TOK_NULL: return "NULL";
        case TOK_FROM: return "FROM";
        case TOK_UPDATE: return "UPDATE";
        case TOK_SET: return "SET";
        case TOK_DELETE: return "DELETE";
        case TOK_JOIN: return "JOIN";
        case TOK_ON: return "ON";
        case TOK_DATABASE: return "DATABASE";
        case TOK_DESCRIBE: return "DESCRIBE";
        default: return "UNKNOWN";
    }
}

int sql_execute(const char *sql_text, const char *dbname) {
    if (dbname && strlen(dbname) > 0) {
        if (db_open(dbname) != 0) {
            fprintf(stderr, "Failed to open database '%s'\n", dbname);
            return DB_ERR;
        }
    }
    lexer_init(sql_text);
    
    AstNode *ast = parse_sql();
    if (!ast) {
        fprintf(stderr, "SQL Parse Error\n");
        return DB_ERR_PARSE;
    }

    int result = DB_OK;

    switch (ast->type) {
        case AST_CREATE_TABLE: {
            TableSchema schema = {0};
            strncpy(schema.name, ast->create_table.table_name, 31);
            schema.column_count = ast->create_table.column_count;
            for (int i = 0; i < schema.column_count; i++) {
                strncpy(schema.columns[i].name, ast->create_table.columns[i].name, 31);
                schema.columns[i].type = ast->create_table.columns[i].type;
                schema.columns[i].not_null = ast->create_table.columns[i].not_null;
                schema.columns[i].unique = ast->create_table.columns[i].unique;
                schema.columns[i].primary_key = ast->create_table.columns[i].primary_key;
            }
            if (catalog_create_table(&schema) != 0) {
                result = DB_ERR;
            } else {
                printf("Table '%s' created.\n", schema.name);
                // auto create indexes for PK and UNIQUE columns
                for (int i = 0; i < schema.column_count; i++) {
                    if (schema.columns[i].primary_key || schema.columns[i].unique) {
                        index_create(schema.name, schema.columns[i].name);
                    }
                }
            }
            break;
        }
        case AST_INSERT: {
            TableSchema *schema = catalog_get_table(ast->insert.table_name);
            if (!schema) {
                fprintf(stderr, "Table '%s' not found\n", ast->insert.table_name);
                result = DB_ERR;
                break;
            }
            if (ast->insert.value_count != schema->column_count) {
                fprintf(stderr, "Insert value count mismatch\n");
                result = DB_ERR;
                free(schema);
                break;
            }
            Row *row = row_create(schema, ast->insert.values);
            // result = table_insert(schema, row);
            //  need the offset of the inserted row for the index.
            // table_insert should return the offset or we should calculate it.
            // actually, table_insert returns 0 on success.
            // maybe modify table_insert to return the offset?
            // or just calculate it: table size before insert.
            
            // actually, let's just use table_insert and then update indexes.
            
            // check uniqueness constraints using indexes BEFORE inserting
            for (int i = 0; i < schema->column_count; i++) {
                if (schema->columns[i].primary_key || schema->columns[i].unique) {
                    char *val = ast->insert.values[i];
                    if (index_lookup(schema->name, schema->columns[i].name, val) != -1) {
                        fprintf(stderr, "Duplicate key error: column '%s' value '%s'\n", schema->columns[i].name, val);
                        result = DB_ERR_CONSTRAINT;
                        break;
                    }
                }
            }
            
            if (result == DB_OK) {
                result = table_insert(schema, row);
                if (result == DB_OK) {
                    printf("1 row inserted.\n");
                    // Update indexes
                    // We need the offset. Let's get it from the file size.
                    char path[1024];
                    extern char db_path[1024];
                    size_t db_path_len = strlen(db_path);
                    size_t name_len = strlen(schema->name);
                    if (db_path_len + 9 + name_len + 4 >= sizeof(path)) {
                        fprintf(stderr, "Table path too long\n");
                        result = DB_ERR;
                        break;
                    }
                    #ifdef _WIN32
                    snprintf(path, sizeof(path), "%s\\tables\\%s.tbl", db_path, schema->name);
                    #else
                    snprintf(path, sizeof(path), "%s/tables/%s.tbl", db_path, schema->name);
                    #endif
                    struct stat st;
                    stat(path, &st);
                    long offset = st.st_size - row_size(schema); // The row we just added
                    
                    for (int i = 0; i < schema->column_count; i++) {
                        if (schema->columns[i].primary_key || schema->columns[i].unique) {
                            index_insert(schema->name, schema->columns[i].name, ast->insert.values[i], offset);
                        }
                    }
                }
            }
            row_free(row);
            free(schema);
            break;
        }
        case AST_SELECT: {
            TableSchema *schema = catalog_get_table(ast->select.table_name);
            if (!schema) {
                fprintf(stderr, "Table '%s' not found\n", ast->select.table_name);
                result = DB_ERR;
                break;
            }
            
            if (ast->select.join_table) {
                TableSchema *schema2 = catalog_get_table(ast->select.join_table);
                if (!schema2) {
                    fprintf(stderr, "Join table '%s' not found\n", ast->select.join_table);
                    result = DB_ERR;
                    free(schema);
                    break;
                }
                result = table_join(schema, schema2, ast->select.join_left_col, ast->select.join_right_col, ast->select.where_column, ast->select.where_value);
                free(schema2);
            } else {
                // Check if we have an index for the WHERE column
                long offset = -1;
                if (ast->select.where_column) {
                    for (int i = 0; i < schema->column_count; i++) {
                        if (strcmp(schema->columns[i].name, ast->select.where_column) == 0) {
                            if (schema->columns[i].primary_key || schema->columns[i].unique) {
                                offset = index_lookup(schema->name, schema->columns[i].name, ast->select.where_value);
                            }
                            break;
                        }
                    }
                }

                if (offset != -1) {
                    // Fast path using index
                    // Let's implement table_select_at_offset in table.c
                    result = table_select_at_offset(schema, ast->select.columns, ast->select.column_count, offset);
                } else {
                    result = table_select(schema, ast->select.columns, ast->select.column_count, ast->select.where_column, ast->select.where_value);
                }
            }
            free(schema);
            break;
        }
        case AST_UPDATE: {
            TableSchema *schema = catalog_get_table(ast->update.table_name);
            if (!schema) {
                fprintf(stderr, "Table '%s' not found\n", ast->update.table_name);
                result = DB_ERR;
                break;
            }
            result = table_update(schema, ast->update.where_column, ast->update.where_value, ast->update.set_column, ast->update.set_value);
            if (result == DB_OK) {
                printf("Table updated.\n");
            }
            free(schema);
            break;
        }
        case AST_DELETE: {
            TableSchema *schema = catalog_get_table(ast->delete_stmt.table_name);
            if (!schema) {
                fprintf(stderr, "Table '%s' not found\n", ast->delete_stmt.table_name);
                result = DB_ERR;
                break;
            }
            result = table_delete(schema, ast->delete_stmt.where_column, ast->delete_stmt.where_value);
            if (result == DB_OK) {
                printf("Rows deleted.\n");
            }
            free(schema);
            break;
        }
        case AST_CREATE_DATABASE: {
            if (db_init(ast->create_database.db_name) != 0) {
                result = DB_ERR_DATABASE_EXISTS;
            } else {
                printf("Database '%s' initialized.\n", ast->create_database.db_name);
            }
            break;
        }
        case AST_DESCRIBE_TABLE: {
            TableSchema *schema = catalog_get_table(ast->describe_table.table_name);
            if (!schema) {
                fprintf(stderr, "Table '%s' not found\n", ast->describe_table.table_name);
                result = DB_ERR_TABLE_NOT_FOUND;
                break;
            }
            printf("Table: %s\n", schema->name);
            printf("%-16s | %-8s | %-8s | %-8s | %-8s\n", "Column", "Type", "NotNull", "Unique", "PK");
            printf("-----------------|----------|----------|----------|----------\n");
            for (int i = 0; i < schema->column_count; i++) {
                printf("%-16s | %-8s | %-8s | %-8s | %-8s\n",
                    schema->columns[i].name,
                    schema->columns[i].type == COL_INT ? "INT" : "TEXT",
                    schema->columns[i].not_null ? "YES" : "NO",
                    schema->columns[i].unique ? "YES" : "NO",
                    schema->columns[i].primary_key ? "YES" : "NO");
            }
            free(schema);
            result = DB_OK;
            break;
        }
        default:
            fprintf(stderr, "Unsupported statement type\n");
            result = DB_ERR;
            break;
    }

    ast_free(ast);
    return result;
}
