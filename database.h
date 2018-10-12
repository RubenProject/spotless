#ifndef DB_OPS_H
#define DB_OPS_H


#include "mongoose/mongoose.h"
#include "queue.h"

#define MEDIA_FOLDER "web_root/media"
#define STMT_MAX 1000
#define NAME_MAX 100


struct summary{
    struct list *column;
    struct list *value;
};



void *db_open(const char *db_path);
void db_close(void **db_handle);
void db_add(void *db, struct summary sum);
void db_update(void *db, const char *cat);
void db_get_tables(void *db, struct list *table_list);
void db_get_summaries(void *db, char *cat, struct summary **sum);


#endif
