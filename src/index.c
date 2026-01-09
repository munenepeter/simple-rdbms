#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "index.h"
#include "storage.h"
#include "errors.h"
#include "catalog.h"

#define INDEX_ENTRY_SIZE (MAX_TEXT_LEN + 1 + sizeof(long))

static void get_index_path(char *path, const char *table, const char *column) {
    size_t db_path_len = strlen(db_path);
    size_t table_len = strlen(table);
    size_t column_len = strlen(column);
    if (db_path_len + 10 + table_len + column_len + 5 >= 1024) {
        fprintf(stderr, "Index path too long\n");
        path[0] = '\0';
        return;
    }
    #ifdef _WIN32
    int n = snprintf(path, 1024, "%s\\indexes\\%s_%s.idx", db_path, table, column);
    #else
    int n = snprintf(path, 1024, "%s/indexes/%s_%s.idx", db_path, table, column);
    #endif

    if (n < 0 || n >= (int)sizeof(path)) {
        fprintf(stderr, "Index path too long\n");
        path[0] = '\0';
        return;
    }
   
}

int index_create(const char *table, const char *column) {
    char path[1024];
    get_index_path(path, table, column);

    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("index_create: open");
        return DB_ERR_FILE_IO;
    }
    close(fd);
    return DB_OK;
}

int index_insert(const char *table, const char *column, const char *key, long offset) {
    char path[1024];
    get_index_path(path, table, column);

    int fd = open(path, O_WRONLY | O_APPEND);
    if (fd < 0) {
        return DB_ERR_INDEX_ERROR;
    }

    char buffer[INDEX_ENTRY_SIZE];
    memset(buffer, 0, INDEX_ENTRY_SIZE);
    strncpy(buffer, key, MAX_TEXT_LEN);
    memcpy(buffer + MAX_TEXT_LEN + 1, &offset, sizeof(long));

    if (write(fd, buffer, INDEX_ENTRY_SIZE) != INDEX_ENTRY_SIZE) {
        perror("index_insert: write");
        close(fd);
        return DB_ERR_FILE_IO;
    }

    close(fd);
    return DB_OK;
}

long index_lookup(const char *table, const char *column, const char *key) {
    char path[1024];
    get_index_path(path, table, column);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    char buffer[INDEX_ENTRY_SIZE];
    while (read(fd, buffer, INDEX_ENTRY_SIZE) == INDEX_ENTRY_SIZE) {
        if (strcmp(buffer, key) == 0) {
            long offset;
            memcpy(&offset, buffer + MAX_TEXT_LEN + 1, sizeof(long));
            close(fd);
            return offset;
        }
    }

    close(fd);
    return -1;
}
