#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include "catalog.h"
#include "storage.h"

typedef struct {
    uint32_t table_count;
} CatalogHeader;


// TODO: fix me: db_path is defined in storage.h as a static char array if STORAGE_IMPLEMENTATION is defined & 
//               we shouldn't define STORAGE_IMPLEMENTATION in this file
extern char db_path[1024];

int catalog_create_table(TableSchema *schema) {
    char path[1024];
    size_t db_path_len = strlen(db_path);
    if (db_path_len + 15 >= sizeof(path)) {
        fprintf(stderr, "Catalog path too long\n");
        return -1;
    }
    snprintf(path, sizeof(path), "%s/catalog.meta", db_path);

    int fd = open(path, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("catalog_create_table: open");
        return -1;
    }

    CatalogHeader hdr = {0};
    ssize_t bytes = read(fd, &hdr, sizeof(hdr));
    if (bytes == 0) {
        // new file, write initial header
        hdr.table_count = 0;
        write(fd, &hdr, sizeof(hdr));
    } else if (bytes < (ssize_t)sizeof(hdr)) {
        // corrupt or partial? resetting
        hdr.table_count = 0;
        lseek(fd, 0, SEEK_SET);
        write(fd, &hdr, sizeof(hdr));
    }

    // append new schema at the end
    lseek(fd, 0, SEEK_END);
    if (write(fd, schema, sizeof(TableSchema)) != sizeof(TableSchema)) {
        perror("catalog_create_table: write schema");
        close(fd);
        return -1;
    }

    // update count at the beginning
    hdr.table_count++;
    lseek(fd, 0, SEEK_SET);
    if (write(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        perror("catalog_create_table: update header");
    }

    close(fd);
    return 0;
}

TableSchema *catalog_get_table(const char *name) {
    (void)name; // unused in this function
    char path[1024];
    size_t db_path_len = strlen(db_path);
    if (db_path_len + 15 >= sizeof(path)) {
        return NULL;
    }
    snprintf(path, sizeof(path), "%s/catalog.meta", db_path);

    int fd = open(path, O_RDONLY);
    if (fd < 0) return NULL;

    CatalogHeader hdr;
    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        close(fd);
        return NULL;
    }

    TableSchema *schema = malloc(sizeof(TableSchema));
    for (uint32_t i = 0; i < hdr.table_count; i++) {
        if (read(fd, schema, sizeof(TableSchema)) != sizeof(TableSchema)) break;
        if (strcmp(schema->name, name) == 0) {
            close(fd);
            return schema;
        }
    }

    free(schema);
    close(fd);
    return NULL;
}
