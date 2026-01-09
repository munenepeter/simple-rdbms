#ifdef _WIN32
#define strcasecmp _stricmp
#define strdup _strdup
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

#undef Y
#undef N
#define Y(x) 1
#define N(x) 0

// Use the standard configuration block from stb_c_lexer.h but with our overrides
#define STB_C_LEX_C_DECIMAL_INTS    Y
#define STB_C_LEX_C_HEX_INTS        Y
#define STB_C_LEX_C_OCTAL_INTS      Y
#define STB_C_LEX_C_DECIMAL_FLOATS  Y
#define STB_C_LEX_C99_HEX_FLOATS    N
#define STB_C_LEX_C_IDENTIFIERS     Y
#define STB_C_LEX_C_DQ_STRINGS      Y
#define STB_C_LEX_C_SQ_STRINGS      Y
#define STB_C_LEX_C_CHARS           N
#define STB_C_LEX_C_COMMENTS        Y
#define STB_C_LEX_CPP_COMMENTS      Y
#define STB_C_LEX_C_COMPARISONS     Y
#define STB_C_LEX_C_LOGICAL         Y
#define STB_C_LEX_C_SHIFTS          Y
#define STB_C_LEX_C_INCREMENTS      Y
#define STB_C_LEX_C_ARROW           Y
#define STB_C_LEX_EQUAL_ARROW       N
#define STB_C_LEX_C_BITWISEEQ       Y
#define STB_C_LEX_C_ARITHEQ         Y
#define STB_C_LEX_PARSE_SUFFIXES    N
#define STB_C_LEX_DECIMAL_SUFFIXES  ""
#define STB_C_LEX_HEX_SUFFIXES      ""
#define STB_C_LEX_OCTAL_SUFFIXES    ""
#define STB_C_LEX_FLOAT_SUFFIXES    ""
#define STB_C_LEX_0_IS_EOF          N
#define STB_C_LEX_INTEGERS_AS_DOUBLES N
#define STB_C_LEX_MULTILINE_DSTRINGS  N
#define STB_C_LEX_MULTILINE_SSTRINGS  N
#define STB_C_LEX_USE_STDLIB        Y
#define STB_C_LEX_DOLLAR_IDENTIFIER Y
#define STB_C_LEX_FLOAT_NO_DECIMAL  Y
#define STB_C_LEX_DEFINE_ALL_TOKEN_NAMES N
#define STB_C_LEX_DISCARD_PREPROCESSOR Y

#define STB_C_LEXER_DEFINITIONS
// --END--

#define STB_C_LEXER_IMPLEMENTATION
#include "libs/stb_c_lexer.h"

static stb_lexer lex;
static char string_store[64 * 1024];

void lexer_init(const char *input) {
    if (!input) return;
    stb_c_lexer_init(&lex, input, input + strlen(input), string_store, sizeof(string_store));
}

Token lexer_peek(void) {
    stb_lexer old_lex = lex;
    Token t = lexer_next();
    // We need to be careful with strdup in lexer_next.
    // If we peek, we might leak if we don't free.
    // But lexer_next strdups for TOK_IDENTIFIER and TOK_STRING.
    lex = old_lex;
    return t;
}

Token lexer_next(void) {
    Token t = { .type = TOK_EOF, .lexeme = NULL };
    
    if (stb_c_lexer_get_token(&lex) == 0) {
        return t;
    }

    switch (lex.token) {
        case CLEX_id: {
            if (strcasecmp(lex.string, "SELECT") == 0) t.type = TOK_SELECT;
            else if (strcasecmp(lex.string, "FROM") == 0) t.type = TOK_FROM;
            else if (strcasecmp(lex.string, "WHERE") == 0) t.type = TOK_WHERE;
            else if (strcasecmp(lex.string, "INSERT") == 0) t.type = TOK_INSERT;
            else if (strcasecmp(lex.string, "INTO") == 0) t.type = TOK_INTO;
            else if (strcasecmp(lex.string, "VALUES") == 0) t.type = TOK_VALUES;
            else if (strcasecmp(lex.string, "CREATE") == 0) t.type = TOK_CREATE;
            else if (strcasecmp(lex.string, "TABLE") == 0) t.type = TOK_TABLE;
            else if (strcasecmp(lex.string, "INT") == 0) t.type = TOK_INT;
            else if (strcasecmp(lex.string, "TEXT") == 0) t.type = TOK_TEXT;
            else if (strcasecmp(lex.string, "PRIMARY") == 0) t.type = TOK_PRIMARY;
            else if (strcasecmp(lex.string, "KEY") == 0) t.type = TOK_KEY;
            else if (strcasecmp(lex.string, "UNIQUE") == 0) t.type = TOK_UNIQUE;
            else if (strcasecmp(lex.string, "NOT") == 0) t.type = TOK_NOT;
            else if (strcasecmp(lex.string, "NULL") == 0) t.type = TOK_NULL;
            else if (strcasecmp(lex.string, "UPDATE") == 0) t.type = TOK_UPDATE;
            else if (strcasecmp(lex.string, "SET") == 0) t.type = TOK_SET;
            else if (strcasecmp(lex.string, "DELETE") == 0) t.type = TOK_DELETE;
            else if (strcasecmp(lex.string, "JOIN") == 0) t.type = TOK_JOIN;
            else if (strcasecmp(lex.string, "ON") == 0) t.type = TOK_ON;
            else if (strcasecmp(lex.string, "DATABASE") == 0) t.type = TOK_DATABASE;
            else if (strcasecmp(lex.string, "DESCRIBE") == 0) t.type = TOK_DESCRIBE;
            else t.type = TOK_IDENTIFIER;
            t.lexeme = strdup(lex.string);
            break;
        }
        case CLEX_intlit: {
            t.type = TOK_NUMBER;
            break;
        }
        case CLEX_dqstring:
        case CLEX_sqstring:
            t.type = TOK_STRING;
            t.lexeme = strdup(lex.string);
            break;
        case ',': t.type = TOK_COMMA; break;
        case ';': t.type = TOK_SEMICOLON; break;
        case '(': t.type = TOK_LPAREN; break;
        case ')': t.type = TOK_RPAREN; break;
        case '=': t.type = TOK_EQUAL; break;
        case '*': t.type = TOK_STAR; break;
        case CLEX_eq: t.type = TOK_EQUAL; break;
        default:
            // return as single char token if it's something we might recognize
            // but the switch missed (e.g. single quotes if not handled by sqstring)
            // But if it's EOF it returns TOK_EOF.
            t.type = TOK_EOF; 
            break;
    }

    if (!t.lexeme && t.type != TOK_EOF) {
        int len = lex.where_lastchar - lex.where_firstchar + 1;
        t.lexeme = malloc(len + 1);
        if (t.lexeme) {
            strncpy(t.lexeme, lex.where_firstchar, len);
            t.lexeme[len] = '\0';
        }
    }
    return t;
}

void lexer_free(void) {
    // string_store is static
}
