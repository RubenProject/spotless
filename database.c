#include "database.h"
#include "sqlite3.h"
#include "queue.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "sys/stat.h"
#include "sys/types.h"


#define CREATE_STMT "CREATE TABLE IF NOT EXISTS video_master "\
                    "(ID INTEGER PRIMARY KEY AUTOINCREMENT, "\
                    "filename TEXT, title TEXT, description TEXT, "\
                    "category TEXT, date_added DATE DEFAULT "\
                    "(datetime('now','localtime')));"


static char *strip_filetype(const char *filename){
    char t[100];
    size_t i;
    strcpy(t, filename);
    for (i = strlen(t) - 1; i > 0; i--){
        if (t[i] == '.'){
            t[i] = '\0';
            break;
        }
        t[i] = '\0';
    }
    return strdup(t);     
}


//open db, create table for each folder in media folder
void *db_open(const char *db_path) {
    sqlite3 *db = NULL;
    struct dirent *dp;
    struct stat stbuf;
    DIR *dfd;
    char filename[NAME_MAX*2];

    if ((dfd = opendir(MEDIA_FOLDER)) == NULL){
        fprintf(stderr, "Could not open %s folder\n Exiting...\n", MEDIA_FOLDER);
        exit(0);
    }

    if (sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                        SQLITE_OPEN_FULLMUTEX, NULL) == SQLITE_OK) {
        fprintf(stdout, CREATE_STMT "\n");
        sqlite3_exec(db, CREATE_STMT, 0, 0, 0);
        while ((dp = readdir(dfd)) != NULL){
            if (dp->d_name[0] == '.')
                continue;
            sprintf(filename, "%s/%s", MEDIA_FOLDER, dp->d_name);
            if (stat(filename, &stbuf) == -1)
                continue;
            if ((stbuf.st_mode & S_IFMT) == S_IFDIR){
                db_update(db, dp->d_name);
            }
        }
    }
    return db;
}


void db_close(void **db_handle) {
  if (db_handle != NULL && *db_handle != NULL) {
    sqlite3_close(*db_handle);
    *db_handle = NULL;
  }
}

int db_get_by_fname(void *db, char *cat, char *fname, struct summary *sum){
    sqlite3_stmt *stmt;
    size_t i, res, rc;
    char sql[STMT_MAX];
    strcpy(sql, "SELECT * FROM video_master WHERE filename=\'");
    strcat(sql, fname);
    strcat(sql, "\' and category=\'");
    strcat(sql, cat);
    strcat(sql, "\';");
    strcpy(sql, "SELECT * FROM video_master;");

    printf("%s\n", sql);
    //XXX: this is recognized as misuse????
    rc = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
    if (rc != SQLITE_OK){
        fprintf(stdout, "error: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    res = 0;
    i = 0;
    sum->column = list_init();
    sum->value = list_init();
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        list_push(sum->column, sqlite3_column_name(stmt, i));
        list_push(sum->value, sqlite3_column_text(stmt, i));
        res = 1;
        i++;
    }
    if (rc != SQLITE_DONE) {
        fprintf(stdout, "error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    return res;
} 


void db_add(void *db, struct summary sum){
    size_t i;
    char stmt[STMT_MAX], *t;
    strcpy(stmt, "INSERT INTO video_master ( ");
    for (i = 0; (t = list_get(sum.column, i)) != NULL; i++) {
        strcat(stmt, t);
        strcat(stmt, ",");
    }
    stmt[strlen(stmt) - 1] = '\0';
    strcat(stmt, ") VALUES ("); 
    for (i = 0; (t = list_get(sum.value, i)) != NULL; i++) {
        strcat(stmt, "\"");
        strcat(stmt, t);
        strcat(stmt, "\",");
    }
    stmt[strlen(stmt) - 1] = '\0';
    strcat(stmt, ");");
    fprintf(stdout, "%s\n", stmt);
    sqlite3_exec(db, stmt, 0, 0, 0);
}


void db_update(void *db, const char *cat){
    struct dirent *dp;
    DIR *dfd;
    char fpath[NAME_MAX*2], *fname;
    sprintf(fpath, "%s/%s", MEDIA_FOLDER, cat);
    if ((dfd = opendir(fpath)) == NULL){
        fprintf(stderr, "Could not open %s folder\n Exiting...", cat);
        exit(0);
    }
    //each file is an entry in database
    struct summary sum;
    while ((dp = readdir(dfd)) != NULL){
        if (dp->d_name[0] == '.')
            continue;
        if (db_get_by_fname(dp, cat, dp->d_name, &sum)){
            //list_free(sum.column);
            //list_free(sum.value);
            continue;
        }
        sum.column = list_init();
        sum.value = list_init();
        list_push(sum.column, "filename");
        list_push(sum.column, "title");
        list_push(sum.column, "description");
        list_push(sum.column, "category");

        fname = strip_filetype(dp->d_name);
        list_push(sum.value, dp->d_name);
        list_push(sum.value, fname);
        list_push(sum.value, "");
        list_push(sum.value, cat);
        db_add(db, sum);
        //list_free(&sum.column);
        //list_free(&sum.value);
    }
}


void db_get_tables(void *db, struct list *table_list){
    sqlite3_stmt *stmt;
    char sql[STMT_MAX] = "SELECT name FROM sqlite_master WHERE type='table'";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stdout, "error: %s\n", sqlite3_errmsg(db));
        return;
    }
    size_t i;
    char ctext[NAME_MAX];
    if (table_list == NULL)
        table_list = list_init();
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        strcpy(ctext, sqlite3_column_text(stmt, 0));
        if (strcmp(ctext, "sqlite_sequence") != 0){
            list_push(table_list, ctext);
        }
    }
    if (rc != SQLITE_DONE) {
        printf("error: ", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}


void db_get_summaries(void *db, char *cat, struct summary **sum){
    sqlite3_stmt *stmt;
    char sql[STMT_MAX];
    strcpy(sql, "SELECT filename video_title description date_added FROM video_master ");
    strcat(sql, "WHERE category=\"");
    strcat(sql, cat);
    strcat(sql, "\";");
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        fprintf(stdout, "error: %s\n", sqlite3_errmsg(db));
        return;
    }
    size_t i;
    //put results in sum

}
