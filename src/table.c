#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "storage.h"
#include "errors.h"

typedef struct {
    TableSchema *schema;
    Row *new_row;
    int conflict;
} UniqueCheckCtx;

static int unique_check_cb(void *row_data, int index, void *ctx) {
    (void)index; // unused - part of callback interface
    UniqueCheckCtx *uctx = (UniqueCheckCtx *)ctx;
    char *existing_ptr = (char *)row_data;
    char *new_ptr = (char *)uctx->new_row->data;

    for (int i = 0; i < uctx->schema->column_count; i++) {
        size_t col_size = (uctx->schema->columns[i].type == COL_INT) ? sizeof(int) : (MAX_TEXT_LEN + 1);
        
        if (uctx->schema->columns[i].unique || uctx->schema->columns[i].primary_key) {
            if (memcmp(existing_ptr, new_ptr, col_size) == 0) {
                uctx->conflict = 1;
                return 1; // stop scan
            }
        }
        existing_ptr += col_size;
        new_ptr += col_size;
    }
    return 0;
}

static size_t get_row_size(TableSchema *schema) {
    size_t size = 0;
    for (int i = 0; i < schema->column_count; i++) {
        size += (schema->columns[i].type == COL_INT) ? sizeof(int) : (MAX_TEXT_LEN + 1);
    }
    return size;
}

int table_insert(TableSchema *schema, Row *row) {
    int fd = storage_table_open(schema->name);
    if (fd < 0) return DB_ERR;

    // unique check
    UniqueCheckCtx uctx = { .schema = schema, .new_row = row, .conflict = 0 };
    storage_table_scan(fd, get_row_size(schema), unique_check_cb, &uctx);

    if (uctx.conflict) {
        fprintf(stderr, "Constraint violation: UNIQUE or PRIMARY KEY conflict\n");
        close(fd);
        return DB_ERR_CONSTRAINT;
    }

    int res = storage_table_append(fd, row->data, get_row_size(schema));
    close(fd);
    return (res == 0) ? DB_OK : DB_ERR;
}

typedef struct {
    TableSchema *schema;
    char **cols;
    int col_count;
    const char *where_col;
    const char *where_val;
} SelectCtx;

static int select_cb(void *row_data, int index, void *ctx) {
    (void)index; // unused - part of callback interface
    SelectCtx *sctx = (SelectCtx *)ctx;
    char *ptr = (char *)row_data;
    
    int match = 1;
    int where_col_idx = -1;

    if (sctx->where_col) {
        match = 0;
        for (int i = 0; i < sctx->schema->column_count; i++) {
            if (strcmp(sctx->schema->columns[i].name, sctx->where_col) == 0) {
                where_col_idx = i;
                break;
            }
        }
    }

    // print/filter
    char *temp_ptr = ptr;
    for (int i = 0; i < sctx->schema->column_count; i++) {
        if (i == where_col_idx) {
            if (sctx->schema->columns[i].type == COL_INT) {
                int val;
                memcpy(&val, temp_ptr, sizeof(int));
                if (val == atoi(sctx->where_val)) match = 1;
            } else {
                if (strcmp(temp_ptr, sctx->where_val) == 0) match = 1;
            }
        }
        temp_ptr += (sctx->schema->columns[i].type == COL_INT) ? sizeof(int) : (MAX_TEXT_LEN + 1);
    }

    if (match) {
        for (int j = 0; j < sctx->col_count; j++) {
            char *col_name = sctx->cols[j];
            int is_star = (strcmp(col_name, "*") == 0);
            
            temp_ptr = ptr;
            for (int i = 0; i < sctx->schema->column_count; i++) {
                size_t col_size = (sctx->schema->columns[i].type == COL_INT) ? sizeof(int) : (MAX_TEXT_LEN + 1);
                if (is_star || strcmp(sctx->schema->columns[i].name, col_name) == 0) {
                    if (sctx->schema->columns[i].type == COL_INT) {
                        int val; memcpy(&val, temp_ptr, sizeof(int));
                        printf("%d", val);
                    } else {
                        printf("'%s'", temp_ptr);
                    }
                    if (is_star) {
                        if (i < sctx->schema->column_count - 1) printf(" | ");
                    } else {
                        if (j < sctx->col_count - 1) printf(" | ");
                    }
                }
                temp_ptr += col_size;
            }
        }
        printf("\n");
    }

    return 0;
}

int table_select(TableSchema *schema, char **cols, int col_count, const char *where_col, const char *where_val) {
    int fd = storage_table_open(schema->name);
    if (fd < 0) return DB_ERR;

    // header
    for (int j = 0; j < col_count; j++) {
        if (strcmp(cols[j], "*") == 0) {
            for (int i = 0; i < schema->column_count; i++) {
                printf("%s", schema->columns[i].name);
                if (i < schema->column_count - 1) printf(" | ");
            }
        } else {
            printf("%s", cols[j]);
            if (j < col_count - 1) printf(" | ");
        }
    }
    printf("\n----------\n");

    SelectCtx sctx = { .schema = schema, .cols = cols, .col_count = col_count, .where_col = where_col, .where_val = where_val };
    storage_table_scan(fd, get_row_size(schema), select_cb, &sctx);

    close(fd);
    return DB_OK;
}

typedef struct {
    TableSchema *schema;
    const char *where_col;
    const char *where_val;
    const char *set_col;
    const char *set_val;
    int fd;
} UpdateCtx;

static int update_cb(void *row_data, int index, void *ctx) {
    UpdateCtx *uctx = (UpdateCtx *)ctx;
    char *ptr = (char *)row_data;
    size_t row_size = get_row_size(uctx->schema);

    int match = 0;
    int where_col_idx = -1;
    if (uctx->where_col) {
        for (int i = 0; i < uctx->schema->column_count; i++) {
            if (strcmp(uctx->schema->columns[i].name, uctx->where_col) == 0) {
                where_col_idx = i;
                break;
            }
        }
    } else {
        match = 1;
    }

    if (where_col_idx != -1) {
        char *temp_ptr = ptr;
        for (int i = 0; i < uctx->schema->column_count; i++) {
            size_t col_size = (uctx->schema->columns[i].type == COL_INT) ? sizeof(int) : (MAX_TEXT_LEN + 1);
            if (i == where_col_idx) {
                if (uctx->schema->columns[i].type == COL_INT) {
                    int val;
                    memcpy(&val, temp_ptr, sizeof(int));
                    if (val == atoi(uctx->where_val)) match = 1;
                } else {
                    if (strcmp(temp_ptr, uctx->where_val) == 0) match = 1;
                }
            }
            temp_ptr += col_size;
        }
    }

    if (match) {
        // prepare new row data
        char *new_row_data = malloc(row_size);
        memcpy(new_row_data, row_data, row_size);

        char *temp_ptr = new_row_data;
        for (int i = 0; i < uctx->schema->column_count; i++) {
            size_t col_size = (uctx->schema->columns[i].type == COL_INT) ? sizeof(int) : (MAX_TEXT_LEN + 1);
            if (strcmp(uctx->schema->columns[i].name, uctx->set_col) == 0) {
                if (uctx->schema->columns[i].type == COL_INT) {
                    int val = atoi(uctx->set_val);
                    memcpy(temp_ptr, &val, sizeof(int));
                } else {
                    strncpy(temp_ptr, uctx->set_val, MAX_TEXT_LEN);
                    temp_ptr[MAX_TEXT_LEN] = '\0';
                }
            }
            temp_ptr += col_size;
        }

        lseek(uctx->fd, index * row_size, SEEK_SET);
        write(uctx->fd, new_row_data, row_size);
        free(new_row_data);
    }

    return 0;
}

int table_update(TableSchema *schema, const char *where_col, const char *where_val, const char *set_col, const char *set_val) {
    int fd = storage_table_open(schema->name);
    if (fd < 0) return DB_ERR;

    UpdateCtx uctx = { .schema = schema, .where_col = where_col, .where_val = where_val, .set_col = set_col, .set_val = set_val, .fd = fd };
    storage_table_scan(fd, get_row_size(schema), update_cb, &uctx);

    close(fd);
    return DB_OK;
}

typedef struct {
    TableSchema *schema;
    const char *where_col;
    const char *where_val;
    int fd;
    int *indices_to_delete;
    int delete_count;
} DeleteCtx;

static int delete_cb(void *row_data, int index, void *ctx) {
    DeleteCtx *dctx = (DeleteCtx *)ctx;
    char *ptr = (char *)row_data;

    int match = 0;
    int where_col_idx = -1;
    if (dctx->where_col) {
        for (int i = 0; i < dctx->schema->column_count; i++) {
            if (strcmp(dctx->schema->columns[i].name, dctx->where_col) == 0) {
                where_col_idx = i;
                break;
            }
        }
    } else {
        match = 1;
    }

    if (where_col_idx != -1) {
        char *temp_ptr = ptr;
        for (int i = 0; i < dctx->schema->column_count; i++) {
            size_t col_size = (dctx->schema->columns[i].type == COL_INT) ? sizeof(int) : (MAX_TEXT_LEN + 1);
            if (i == where_col_idx) {
                if (dctx->schema->columns[i].type == COL_INT) {
                    int val;
                    memcpy(&val, temp_ptr, sizeof(int));
                    if (val == atoi(dctx->where_val)) match = 1;
                } else {
                    if (strcmp(temp_ptr, dctx->where_val) == 0) match = 1;
                }
            }
            temp_ptr += col_size;
        }
    }

    if (match) {
        dctx->indices_to_delete[dctx->delete_count++] = index;
    }

    return 0;
}

int table_delete(TableSchema *schema, const char *where_col, const char *where_val) {
    int fd = storage_table_open(schema->name);
    if (fd < 0) return DB_ERR;

    size_t row_size = get_row_size(schema);
    struct stat st;
    fstat(fd, &st);
    int total_rows = st.st_size / row_size;

    int *indices = malloc(sizeof(int) * total_rows);
    DeleteCtx dctx = { .schema = schema, .where_col = where_col, .where_val = where_val, .fd = fd, .indices_to_delete = indices, .delete_count = 0 };
    
    storage_table_scan(fd, row_size, delete_cb, &dctx);

    // delete in reverse order to minimize impact of swap-and-truncate? 
    // actually, each delete changes the last row. if i delete indices in any order, i must be careful.
    // if i delete from end to beginning, it might be easier.
    // but swap-and-truncate means the 'last row' moves to the 'deleted row' index.
    
    for (int i = dctx.delete_count - 1; i >= 0; i--) {
        int idx = dctx.indices_to_delete[i];
        
        // refresh total_rows for each deletion
        fstat(fd, &st);
        int current_total = st.st_size / row_size;
        if (current_total == 0) break;

        if (idx < current_total - 1) {
            // swap last row to this position
            char *last_row = malloc(row_size);
            lseek(fd, (current_total - 1) * row_size, SEEK_SET);
            read(fd, last_row, row_size);
            
            lseek(fd, idx * row_size, SEEK_SET);
            write(fd, last_row, row_size);
            free(last_row);
        }
        
        // truncate file
        #ifdef _WIN32
        _chsize(fd, (current_total - 1) * row_size);
        #else
        ftruncate(fd, (current_total - 1) * row_size);
        #endif
    }

    free(indices);
    close(fd);
    return DB_OK;
}

typedef struct {
    TableSchema *s1;
    TableSchema *s2;
    const char *col1;
    const char *col2;
    const char *where_col;
    const char *where_val;
    void *row1_data;
} JoinCtx;

static int join_inner_cb(void *row2_data, int index, void *ctx) {
    (void)index; // unused - part of callback interface
    JoinCtx *jctx = (JoinCtx *)ctx;
    
    // find col1 in row1
    int col1_idx = -1;
    for (int i = 0; i < jctx->s1->column_count; i++) {
        if (strcmp(jctx->s1->columns[i].name, jctx->col1) == 0) {
            col1_idx = i;
            break;
        }
    }

    // find col2 in row2
    int col2_idx = -1;
    for (int i = 0; i < jctx->s2->column_count; i++) {
        if (strcmp(jctx->s2->columns[i].name, jctx->col2) == 0) {
            col2_idx = i;
            break;
        }
    }

    if (col1_idx == -1 || col2_idx == -1) return -1;

    char *p1 = (char *)jctx->row1_data;
    for (int i = 0; i < col1_idx; i++) {
        p1 += (jctx->s1->columns[i].type == COL_INT) ? sizeof(int) : (MAX_TEXT_LEN + 1);
    }

    char *p2 = (char *)row2_data;
    for (int i = 0; i < col2_idx; i++) {
        p2 += (jctx->s2->columns[i].type == COL_INT) ? sizeof(int) : (MAX_TEXT_LEN + 1);
    }

    int match = 0;
    if (jctx->s1->columns[col1_idx].type == COL_INT) {
        int v1, v2;
        memcpy(&v1, p1, sizeof(int));
        memcpy(&v2, p2, sizeof(int));
        if (v1 == v2) match = 1;
    } else {
        if (strcmp(p1, p2) == 0) match = 1;
    }

    if (match) {
        // print combined row
        char *ptr = (char *)jctx->row1_data;
        for (int i = 0; i < jctx->s1->column_count; i++) {
            if (jctx->s1->columns[i].type == COL_INT) {
                int val; memcpy(&val, ptr, sizeof(int));
                printf("%d", val);
                ptr += sizeof(int);
            } else {
                printf("'%s'", ptr);
                ptr += MAX_TEXT_LEN + 1;
            }
            printf(" | ");
        }
        ptr = (char *)row2_data;
        for (int i = 0; i < jctx->s2->column_count; i++) {
            if (jctx->s2->columns[i].type == COL_INT) {
                int val; memcpy(&val, ptr, sizeof(int));
                printf("%d", val);
                ptr += sizeof(int);
            } else {
                printf("'%s'", ptr);
                ptr += MAX_TEXT_LEN + 1;
            }
            if (i < jctx->s2->column_count - 1) printf(" | ");
        }
        printf("\n");
    }

    return 0;
}

static int join_outer_cb(void *row1_data, int index, void *ctx) {
    (void)index; // unused - part of callback interface
    JoinCtx *jctx = (JoinCtx *)ctx;
    jctx->row1_data = row1_data;

    // WHERE on row1
    if (jctx->where_col) {
        int where_idx = -1;
        for (int i = 0; i < jctx->s1->column_count; i++) {
            if (strcmp(jctx->s1->columns[i].name, jctx->where_col) == 0) {
                where_idx = i;
                break;
            }
        }
        if (where_idx != -1) {
            char *ptr = (char *)row1_data;
            for (int i = 0; i < where_idx; i++) {
                ptr += (jctx->s1->columns[i].type == COL_INT) ? sizeof(int) : (MAX_TEXT_LEN + 1);
            }
            if (jctx->s1->columns[where_idx].type == COL_INT) {
                int val; memcpy(&val, ptr, sizeof(int));
                if (val != atoi(jctx->where_val)) return 0;
            } else {
                if (strcmp(ptr, jctx->where_val) != 0) return 0;
            }
        }
    }

    int fd2 = storage_table_open(jctx->s2->name);
    if (fd2 < 0) return -1;
    storage_table_scan(fd2, get_row_size(jctx->s2), join_inner_cb, jctx);
    close(fd2);
    return 0;
}

int table_join(TableSchema *s1, TableSchema *s2, const char *col1, const char *col2, const char *where_col, const char *where_val) {
    int fd1 = storage_table_open(s1->name);
    if (fd1 < 0) return DB_ERR;

    // Header
    for (int i = 0; i < s1->column_count; i++) printf("%s.%s | ", s1->name, s1->columns[i].name);
    for (int i = 0; i < s2->column_count; i++) {
        printf("%s.%s", s2->name, s2->columns[i].name);
        if (i < s2->column_count - 1) printf(" | ");
    }
    printf("\n");

    JoinCtx jctx = { .s1 = s1, .s2 = s2, .col1 = col1, .col2 = col2, .where_col = where_col, .where_val = where_val };
    storage_table_scan(fd1, get_row_size(s1), join_outer_cb, &jctx);

    close(fd1);
    return DB_OK;
}

int table_select_at_offset(TableSchema *schema, char **cols, int col_count, long offset) {
    int fd = storage_table_open(schema->name);
    if (fd < 0) return DB_ERR;

    size_t row_size = get_row_size(schema);
    char *buffer = malloc(row_size);
    if (!buffer) {
        close(fd);
        return DB_ERR;
    }

    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("table_select_at_offset: lseek");
        free(buffer);
        close(fd);
        return DB_ERR;
    }

    if (read(fd, buffer, row_size) != (ssize_t)row_size) {
        fprintf(stderr, "Failed to read row at offset %ld\n", offset);
        free(buffer);
        close(fd);
        return DB_ERR;
    }

    // header
    for (int j = 0; j < col_count; j++) {
        if (strcmp(cols[j], "*") == 0) {
            for (int i = 0; i < schema->column_count; i++) {
                printf("%s", schema->columns[i].name);
                if (i < schema->column_count - 1) printf(" | ");
            }
        } else {
            printf("%s", cols[j]);
            if (j < col_count - 1) printf(" | ");
        }
    }
    printf("\n----------\n");

    // print row with filtering
    for (int j = 0; j < col_count; j++) {
        char *col_name = cols[j];
        int is_star = (strcmp(col_name, "*") == 0);
        
        char *temp_ptr = buffer;
        for (int i = 0; i < schema->column_count; i++) {
            size_t col_size = (schema->columns[i].type == COL_INT) ? sizeof(int) : (MAX_TEXT_LEN + 1);
            if (is_star || strcmp(schema->columns[i].name, col_name) == 0) {
                if (schema->columns[i].type == COL_INT) {
                    int val; memcpy(&val, temp_ptr, sizeof(int));
                    printf("%d", val);
                } else {
                    printf("'%s'", temp_ptr);
                }
                if (is_star) {
                    if (i < schema->column_count - 1) printf(" | ");
                } else {
                    if (j < col_count - 1) printf(" | ");
                }
            }
            temp_ptr += col_size;
        }
    }
    printf("\n");

    free(buffer);
    close(fd);
    return DB_OK;
}
