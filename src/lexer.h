typedef enum {
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,
    TOK_COMMA,
    TOK_SEMICOLON,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_EQUAL,
    TOK_STAR,

    TOK_SELECT,
    TOK_INSERT,
    TOK_CREATE,
    TOK_TABLE,
    TOK_INTO,
    TOK_VALUES,
    TOK_WHERE,
    TOK_INT,
    TOK_TEXT,
    TOK_PRIMARY,
    TOK_KEY,
    TOK_UNIQUE,
    TOK_NOT,
    TOK_NULL,
    TOK_FROM,
    TOK_UPDATE,
    TOK_SET,
    TOK_DELETE,
    TOK_JOIN,
    TOK_ON,
    TOK_DATABASE,
    TOK_DESCRIBE
} TokenType;

typedef struct {
    TokenType type;
    char      *lexeme;
} Token;

void lexer_init(const char *input);
Token lexer_next(void);
Token lexer_peek(void);
void lexer_free(void);