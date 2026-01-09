#include <stdio.h>
#include <string.h>
#include "repl.h"
#include "sql.h"
#include "storage.h"
#include "util.h"

int main(int argc, char **argv) {
    if (argc < 1) {
        return 1;
    }

    if (argc >= 2 && strcmp(argv[1], "-h") == 0) {
        print_help();
        return 0;
    }

    if (argc == 1) {
        repl_run("");
        db_close();
        return 0;
    }

    if (argc == 2) {
        const char *arg = argv[1];
        // If it looks like SQL (ends with ; or contains space), execute it.
        if (strchr(arg, ' ') || (strlen(arg) > 0 && arg[strlen(arg)-1] == ';')) {
            if (sql_execute(arg, "") != 0) {
                return 1;
            }
            return 0;
        } else {
            // Likely a database name.
            repl_run(arg);
            db_close();
            return 0;
        }
    }

    if (argc >= 3) {
        const char *dbname = argv[1];
        const char *sql = argv[2];
        
        if (sql_execute(sql, dbname) != 0) {
            db_close();
            return 1;
        }
        db_close();
        return 0;
    }

    return 0;
}
