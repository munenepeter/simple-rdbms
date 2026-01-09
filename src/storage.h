#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define mkdir(path, mode) _mkdir(path)
#define fsync(fd) _commit(fd)
#endif
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define DB_MAGIC 0x41414141

#ifndef STORAGE_H
#define STORAGE_H

typedef struct {
    unsigned int magic;
    int version;
    int page_size;
} DbHeader;

extern char db_path[1024];

int db_init(const char *path);
int db_open(const char *path);
void db_close(void);
int storage_table_open(const char *name);
int storage_table_append(int fd, void *row, size_t size);
int storage_table_scan(int fd,
                        size_t row_size,
                        int (*cb)(void *row, int index, void *ctx),
                        void *ctx);
#endif



#if defined(STORAGE_IMPLEMENTATION)

// this is going to eat my head
char db_path[1024];

int db_init(const char *name) {
    struct stat st;
    char full_path[256];

    // ensure data directory exists
    if (stat("data", &st) != 0) {
        mkdir("data", 0755);
    }

    snprintf(full_path, sizeof(full_path), "data/%s", name);

    if (stat(full_path, &st) == 0) {
        fprintf(stderr, "Database '%s' already exists\n", name);
        return -1;
    }

    if (mkdir(full_path, 0755) != 0) {
        perror("mkdir database");
        return -1;
    }

    // change directory to the database path to create subdirs and files easily
    //  usings full paths.
    char sub_path[512];
    
    snprintf(sub_path, sizeof(sub_path), "%s/tables", full_path);
    mkdir(sub_path, 0755);

    snprintf(sub_path, sizeof(sub_path), "%s/indexes", full_path);
    mkdir(sub_path, 0755);

    snprintf(db_path, sizeof(db_path), "%s", full_path);

    snprintf(sub_path, sizeof(sub_path), "%s/db.meta", full_path);
    int fd = open(sub_path, O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        perror("open db.meta");
        return -1;
    }

    DbHeader hdr = {
        .magic = DB_MAGIC,
        .version = 1,
        .page_size = 4096
    };

    if (write(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        perror("write db.meta");
        close(fd);
        return -1;
    }
    fsync(fd);
    close(fd);

    return 0;
}

int db_open(const char *name) {
    int fd;
    DbHeader hdr;
    char meta_path[512];
    struct stat st;
    const char *candidate_paths[] = {
        "data/%s",
        "../data/%s",
        "../../data/%s"
    };
    int found = 0;

    if (strncmp(name, "data/", 5) == 0) {
        snprintf(db_path, sizeof(db_path), "%s", name);
        snprintf(meta_path, sizeof(meta_path), "%s/db.meta", db_path);
        if (stat(meta_path, &st) == 0) {
            found = 1;
        }
    } else {
        // try multiple possible locations for the data directory
        // so that i can calling from deferent paths
        for (size_t i = 0; i < sizeof(candidate_paths) / sizeof(candidate_paths[0]); i++) {
            snprintf(db_path, sizeof(db_path), candidate_paths[i], name);
            snprintf(meta_path, sizeof(meta_path), "%s/db.meta", db_path);
            if (stat(meta_path, &st) == 0) {
                found = 1;
                break;
            }
        }
    }

    if (!found) {
        fprintf(stderr, "Database '%s' not found. Tried:\n", name);
        if (strncmp(name, "data/", 5) != 0) {
            for (size_t i = 0; i < sizeof(candidate_paths) / sizeof(candidate_paths[0]); i++) {
                char try_path[512];
                snprintf(try_path, sizeof(try_path), candidate_paths[i], name);
                fprintf(stderr, "  - %s/db.meta\n", try_path);
            }
        } else {
            fprintf(stderr, "  - %s/db.meta\n", name);
        }
        return -1;
    }

    fd = open(meta_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open database metadata file: %s\n", meta_path);
        perror("db_open");
        return -1;
    }

    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        perror("read db.meta");
        close(fd);
        return -1;
    }
    close(fd);

    if (hdr.magic != DB_MAGIC) {
        fprintf(stderr, "Invalid database file: %s\n", meta_path);
        return -1;
    }

    if (hdr.version != 1) {
        fprintf(stderr, "Unsupported database version: %d\n", hdr.version);
        return -1;
    }

    return 0;
}

void db_close(void) {
    // Currently no global state needs cleanup besides db_path
    db_path[0] = '\0';
}

int storage_table_open(const char *name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/tables/%s.tbl", db_path, name);
    
    int fd = open(path, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("open table");
        return -1;
    }
    return fd;
}

/**
 * @brief Append a row to the end of a table.
 *
 * @param fd The file descriptor of the table.
 * @param row The row to append.
 * @param size The size of the row.
 *
 * @return 0 on success, -1 on error.
 */
/*@{*/
int storage_table_append(int fd, void *row, size_t size) {
    lseek(fd, 0, SEEK_END);
    if (write(fd, row, size) != (ssize_t)size) {
        perror("write row");
        return -1;
    }
    fsync(fd);
    return 0;
}

/**
 * @brief Scan a table and apply a callback to each row.
 *
 * @param fd The file descriptor of the table.
 * @param row_size The size of each row in the table.
 * @param cb The callback to apply to each row.
 * @param ctx The context to pass to the callback.
 *
 * @return The number of rows processed, or -1 if an error occurred.
 */
int storage_table_scan(int fd, size_t row_size, int (*cb)(void *row, int index, void *ctx), void *ctx) {
    if (row_size == 0) return -1;
    
    char *buffer = malloc(row_size);
    if (!buffer) return -1;
    
    lseek(fd, 0, SEEK_SET);
    int count = 0;
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, row_size)) == (ssize_t)row_size) {
        if (cb(buffer, count, ctx) != 0) break;
        count++;
    }
    
    free(buffer);
    return count;
}
#endif