#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "repl.h"
#include "sql.h"

#define MAX_INPUT_SIZE 1024

void repl_init(void) {
    printf("Simple RDBMS v0.1\n");
    printf("Type 'exit' to quit.\n");
}

void repl_run(const char *dbname) {
    char input[MAX_INPUT_SIZE];
    char statement[MAX_INPUT_SIZE];
    int stmt_pos = 0;
    
    repl_init();
    printf("Using database '%s'\n", dbname);

    while (true) {
        if (stmt_pos == 0) {
            printf("db> ");
        } else {
            printf("... ");
        }
        
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }

        // remove newlines
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
            len--;
        }

        //allow exits
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // add to current statement
        if (stmt_pos + len + 1 < MAX_INPUT_SIZE) {
            if (stmt_pos > 0) {
                statement[stmt_pos++] = ' ';
            }
            strcpy(statement + stmt_pos, input);
            stmt_pos += len;
        } else {
            fprintf(stderr, "Input too long, clearing buffer.\n");
            stmt_pos = 0;
            continue;
        }

        // Check for semicolon
        if (stmt_pos > 0 && statement[stmt_pos-1] == ';') {
            statement[stmt_pos] = '\0'; // null termination
            sql_execute(statement, dbname);
            stmt_pos = 0;
        }
    }
}
