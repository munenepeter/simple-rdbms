#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

AstNode *parse_sql(void);
void ast_free(AstNode *node);

#endif
