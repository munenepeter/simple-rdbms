#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef UTIL_H
#define UTIL_H

void *xmalloc(size_t sz);
char *strdup_safe(const char *s);
char *strndup_safe(const char *s, size_t n);

void print_help();

#endif


#if defined(UTIL_IMPLEMENTATION)
void *xmalloc(size_t size){
    void *p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}
char *strdup_safe(const char *string){
    char *p = strdup(string);
    if (p == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}
char *strndup_safe(const char *s, size_t n){
    size_t len = strlen(s);
    if (len > n) len = n;
    char *p = malloc(len + 1);
    if (p == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    memcpy(p, s, len);
    p[len] = '\0';
    return p;
}



//print help
void print_help(){
    printf("Simple RDBMS - A very simple relational database\n\n");
    printf("Usage:\n");
    printf("  db [dbname]             - Start REPL (interactive mode)\n");
    printf("  db [dbname] \"SQL;\"      - Execute a single SQL statement\n");
    printf("  db -h                   - Show this help message\n\n");
    printf("Examples:\n");
    printf("  db testdb\n");
    printf("  db testdb \"CREATE TABLE users (id INT, name TEXT);\"\n");
}
#endif // UTIL_IMPLEMENTATION
