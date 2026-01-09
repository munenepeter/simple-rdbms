#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
#include "util.h"

static Token current;

static void advance(void) {
    current = lexer_next();
}

static void expect(TokenType type) {
    if (current.type != type) {
        fprintf(stderr, "Parse error: unexpected token '%s'\n",
                current.lexeme ? current.lexeme : "<eof>");
        exit(1);
    }
    advance();
}

static char *expect_identifier(void) {
    if (current.type != TOK_IDENTIFIER) {
        fprintf(stderr, "Expected identifier\n");
        exit(1);
    }
    char *s = strdup_safe(current.lexeme);
    advance();
    return s;
}

static AstNode *parse_create_database(void) {
    AstNode *node = xmalloc(sizeof(*node));
    node->type = AST_CREATE_DATABASE;

    expect(TOK_CREATE);
    expect(TOK_DATABASE);

    node->create_database.db_name = expect_identifier();
    return node;
}

static AstNode *parse_describe_table(void) {
    AstNode *node = xmalloc(sizeof(*node));
    node->type = AST_DESCRIBE_TABLE;

    expect(TOK_DESCRIBE);

    node->describe_table.table_name = expect_identifier();
    return node;
}


static AstNode *parse_create_table(void) {
    AstNode *node = xmalloc(sizeof(*node));
    node->type = AST_CREATE_TABLE;

    expect(TOK_CREATE);
    expect(TOK_TABLE);

    node->create_table.table_name = expect_identifier();
    expect(TOK_LPAREN);

    node->create_table.columns = xmalloc(sizeof(ColumnDef) * 32);
    node->create_table.column_count = 0;

    while (current.type != TOK_RPAREN) {
        ColumnDef *col =
            &node->create_table.columns[node->create_table.column_count++];

        memset(col, 0, sizeof(*col));
        col->name = expect_identifier();

        if (current.type == TOK_INT) {
            col->type = COL_INT;
            advance();
        } else if (current.type == TOK_TEXT) {
            col->type = COL_TEXT;
            advance();
        } else {
            fprintf(stderr, "Unknown column type: %s (type %d)\n", current.lexeme, current.type);
            exit(1);
        }

        /* constraints (optional) */
        while (current.type == TOK_PRIMARY ||
               current.type == TOK_UNIQUE ||
               current.type == TOK_NOT) {

            if (current.type == TOK_PRIMARY) {
                advance();
                expect(TOK_KEY);
                col->primary_key = 1;
                col->unique = 1;
            } else if (current.type == TOK_UNIQUE) {
                advance();
                col->unique = 1;
            } else if (current.type == TOK_NOT) {
                advance();
                expect(TOK_NULL);
                col->not_null = 1;
            }
        }

        if (current.type == TOK_COMMA)
            advance();
    }

    expect(TOK_RPAREN);
    return node;
}

static AstNode *parse_insert(void)
{
    AstNode *node = xmalloc(sizeof(*node));
    node->type = AST_INSERT;

    expect(TOK_INSERT);
    expect(TOK_INTO);

    node->insert.table_name = expect_identifier();
    expect(TOK_VALUES);
    expect(TOK_LPAREN);

    node->insert.values = xmalloc(sizeof(char *) * 32);
    node->insert.value_count = 0;

    while (current.type != TOK_RPAREN) {
        if (current.type != TOK_NUMBER &&
            current.type != TOK_STRING) {
            fprintf(stderr, "Expected literal value\n");
            exit(1);
        }

        node->insert.values[node->insert.value_count++] =
            strdup_safe(current.lexeme);

        advance();

        if (current.type == TOK_COMMA)
            advance();
    }

    expect(TOK_RPAREN);
    return node;
}


static AstNode *parse_select(void) {
    AstNode *node = xmalloc(sizeof(*node));
    node->type = AST_SELECT;

    expect(TOK_SELECT);
    
    node->select.column_count = 0;
    node->select.columns = xmalloc(sizeof(char *) * 32);

    if (current.type == TOK_STAR) {
        node->select.columns[node->select.column_count++] = strdup_safe("*");
        advance();
    } else {
        while (1) {
            node->select.columns[node->select.column_count++] = expect_identifier();
            if (current.type == TOK_COMMA) {
                advance();
            } else {
                break;
            }
        }
    }
    
    expect(TOK_FROM);

    node->select.table_name = expect_identifier();
    node->select.where_column = NULL;
    node->select.where_value = NULL;
    node->select.join_table = NULL;
    node->select.join_left_col = NULL;
    node->select.join_right_col = NULL;

    if (current.type == TOK_JOIN) {
        advance();
        node->select.join_table = expect_identifier();
        expect(TOK_ON);
        node->select.join_left_col = expect_identifier();
        expect(TOK_EQUAL);
        node->select.join_right_col = expect_identifier();
    }

    if (current.type == TOK_WHERE) {
        advance();
        node->select.where_column = expect_identifier();
        expect(TOK_EQUAL);

        if (current.type != TOK_NUMBER &&
            current.type != TOK_STRING) {
            fprintf(stderr, "Expected literal in WHERE\n");
            exit(1);
        }

        node->select.where_value = strdup_safe(current.lexeme);
        advance();
    }

    return node;
}

static AstNode *parse_update(void) {
    AstNode *node = xmalloc(sizeof(*node));
    node->type = AST_UPDATE;

    expect(TOK_UPDATE);
    node->update.table_name = expect_identifier();
    expect(TOK_SET);
    node->update.set_column = expect_identifier();
    expect(TOK_EQUAL);

    if (current.type != TOK_NUMBER &&
        current.type != TOK_STRING) {
        fprintf(stderr, "Expected literal in SET\n");
        exit(1);
    }
    node->update.set_value = strdup_safe(current.lexeme);
    advance();

    node->update.where_column = NULL;
    node->update.where_value = NULL;

    if (current.type == TOK_WHERE) {
        advance();
        node->update.where_column = expect_identifier();
        expect(TOK_EQUAL);

        if (current.type != TOK_NUMBER &&
            current.type != TOK_STRING) {
            fprintf(stderr, "Expected literal in WHERE\n");
            exit(1);
        }
        node->update.where_value = strdup_safe(current.lexeme);
        advance();
    }

    return node;
}

static AstNode *parse_delete(void) {
    AstNode *node = xmalloc(sizeof(*node));
    node->type = AST_DELETE;

    expect(TOK_DELETE);
    expect(TOK_FROM);
    node->delete_stmt.table_name = expect_identifier();

    node->delete_stmt.where_column = NULL;
    node->delete_stmt.where_value = NULL;

    if (current.type == TOK_WHERE) {
        advance();
        node->delete_stmt.where_column = expect_identifier();
        expect(TOK_EQUAL);

        if (current.type != TOK_NUMBER &&
            current.type != TOK_STRING) {
            fprintf(stderr, "Expected literal in WHERE\n");
            exit(1);
        }
        node->delete_stmt.where_value = strdup_safe(current.lexeme);
        advance();
    }

    return node;
}

AstNode *parse_sql(void) {
    advance();

    if (current.type == TOK_CREATE) {
        Token next = lexer_peek();
        if (next.type == TOK_TABLE) {
            return parse_create_table();
        } else if (next.type == TOK_DATABASE) {
            return parse_create_database();
        } else {
            fprintf(stderr, "Expected TABLE or DATABASE after CREATE\n");
            exit(1);
        }
    } else if (current.type == TOK_DESCRIBE) {
        return parse_describe_table();
    } else if (current.type == TOK_SELECT) {
        return parse_select();
    } else if (current.type == TOK_INSERT) {
        return parse_insert();
    } else if (current.type == TOK_UPDATE) {
        return parse_update();
    } else if (current.type == TOK_DELETE) {
        return parse_delete();
    }

    fprintf(stderr, "Unknown statement: %s\n", current.lexeme ? current.lexeme : "NULL");
    exit(1);
}


void ast_free(AstNode *node){
    if (!node) return;

    free(node);
}
