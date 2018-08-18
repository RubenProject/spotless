#ifndef DB_OPS_H
#define DB_OPS_H


#include "mongoose/mongoose.h"
#include "list.h"

#define MEDIA_FOLDER "web_root/media"
#define COLUMN_MAX 4
#define STMT_MAX 500
#define NAME_MAX 100


struct sql_data {
    char column[10][NAME_MAX];
    char value[10][NAME_MAX];
    size_t count;
};




void *db_open(const char *db_path);
void db_close(void **db_handle);
void db_add(void *db, const char *tablename, struct sql_data *data);
int db_get_by_fname(void *db, const char *tablename, const char *fname, struct sql_data *p);
void db_get_tables(void *db, struct str_list **table_list);


#endif
