#ifndef DB_OPS_H
#define DB_OPS_H


#include "mongoose/mongoose.h"
#include "queue.h"
#include "sqlite3.h"

#define MEDIA_FOLDER "web_root/media"
#define STMT_MAX 1000
#define NAME_MAX 100


struct summary{
    struct list *column;
    struct list *value;
};

struct vid_sum{
    char location[1000];
    char title[1000];
    char description[1000];
    char category[1000];
    //probably some hash ? (md5??)
    //location of thumb
};


void *db_open(const char *db_path);
void db_close(void **db_handle);
void db_get_tables(void *db, struct list *table_list);
void db_get_all_sum(sqlite3 *db, char *table, struct vid_sum **sum, int *n);


#endif
