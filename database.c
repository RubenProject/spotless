#include "database.h"
#include "sqlite3.h"
#include "list.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "sys/stat.h"
#include "sys/types.h"


#define CREATE_STMT_0 "CREATE TABLE IF NOT EXISTS "
#define CREATE_STMT_1 "(ID INTEGER PRIMARY KEY AUTOINCREMENT, filename TEXT, title TEXT, description TEXT, category TEXT, date_added DATE DEFAULT (datetime('now','localtime')));"



//open db, create table for each folder in media folder
void *db_open(const char *db_path) {
    sqlite3 *db = NULL;
    struct dirent *dp;
    struct stat stbuf;
    DIR *dfd;
    const char *dir = MEDIA_FOLDER;
    char stmt[STMT_MAX];
    char filename[NAME_MAX];
    char *table_name;

    if ((dfd = opendir(dir)) == NULL){
        fprintf(stderr, "Could not open %s folder\n Exiting...", dir);
        exit(0);
    }

    if (sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                        SQLITE_OPEN_FULLMUTEX, NULL) == SQLITE_OK) {
        while ((dp = readdir(dfd)) != NULL){
            sprintf(filename, "%s/%s",dir, dp->d_name);
            if (dp->d_name[0] == '.')
                continue;
            if (stat(filename, &stbuf) == -1)
                continue;
            if ((stbuf.st_mode & S_IFMT ) == S_IFDIR){
                table_name = dp->d_name;
                sprintf(stmt, "%s%s%s", CREATE_STMT_0, table_name, CREATE_STMT_1);
                fprintf(stdout, "%s\n", stmt);
                sqlite3_exec(db, stmt, 0, 0, 0);
                //db_update_table(db, table_name);
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

int db_get_by_fname(void *db, const char *tablename, const char *fname, struct sql_data *p){
    sqlite3_stmt *stmt;
    char sql[STMT_MAX] = "SELECT * FROM ";
    strcat(sql, tablename);
    strcat(sql, " WHERE filename=");
    sprintf(sql, "%s'%s'", sql, fname);
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("error: ", sqlite3_errmsg(db));
        return;
    }
    int res = 0;
    size_t i;
    p->count = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        for (i = 1; i < COLUMN_MAX; i++){
            strcpy(p->column[i], sqlite3_column_name(stmt, i));
            strcpy(p->value[i], sqlite3_column_text(stmt, i));
            p->count++;
            res = 1;
        }
        continue;
    }
    if (rc != SQLITE_DONE) {
        printf("error: ", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    return res;
} 


int db_get_by_tag(void *db, const char *tablename, const char *category, struct sql_data *p){
    return 0;
}


void db_add(void *db, const char *tablename, struct sql_data *data){
    size_t i;
    char stmt[STMT_MAX] = "INSERT INTO ";
    sprintf(stmt, "%s %s (", stmt, tablename);
    for (i = 0; i < data->count; i++) {
        sprintf(stmt, "%s%s,", stmt, data->column[i]);
    }
    stmt[strlen(stmt) - 1] = '\0';
    strcat(stmt, ") VALUES ("); 
    for (i = 0; i < data->count; i++) {
        sprintf(stmt, "%s'%s',", stmt, data->value[i]);
    }
    stmt[strlen(stmt) - 1] = '\0';
    strcat(stmt, ");");
    fprintf(stdout, "%s\n", stmt);
    sqlite3_exec(db, stmt, 0, 0, 0);
}


void parse_file(struct dirent *dp, struct sql_data *data){
    //filename
    strcpy(data->column[0], "filename");
    sprintf(data->value[0], "'%s'", dp->d_name);
    //video title
    char t[100];
    strcpy(t, dp->d_name);
    int i;
    for (i = strlen(t) - 1; i >= 0; i--){
        if (t[i] == '.'){
            t[i] = '\0';
            break;
        }
        t[i] = '\0';
    }
    strcpy(data->column[1], "title");
    sprintf(data->value[1], "%s", t);
    //description
    strcpy(data->column[2], "description");
    strcpy(data->value[2], "no description given");
    //category
    strcpy(data->column[3], "category");
    strcpy(data->value[3], "none");
    //generate a thumbnail
    data->count = 4;
}


void db_update_table(void *db, const char *tablename){
    //loop through folder
    struct dirent *dp;
    DIR *dfd;

    char dir[NAME_MAX];
    sprintf(dir, "%s/%s", MEDIA_FOLDER, tablename);
    if ((dfd = opendir(dir)) == NULL){
        fprintf(stderr, "Could not open %s folder\n Exiting...", dir);
        exit(0);
    }
    //each file put entry in database
    struct sql_data *data = malloc(sizeof(struct sql_data));
    while ((dp = readdir(dfd)) != NULL){
        if (dp->d_name[0] != '.'){
            if (!db_get_by_fname(dp, tablename, dp->d_name, data) == 0) {
                parse_file(dp, data);
                db_add(db, tablename, data);
            }
        }
    }
}


void db_get_tables(void *db, struct str_list **table_list){
    sqlite3_stmt *stmt;
    char sql[STMT_MAX] = "SELECT name FROM sqlite_master WHERE type='table'";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("error: ", sqlite3_errmsg(db));
        return;
    }
    size_t i;
    char ctext[NAME_MAX];
    if (*table_list == NULL)
        *table_list = list_create();
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        strcpy(ctext, sqlite3_column_text(stmt, 0));
        if (strcmp(ctext, "sqlite_sequence") != 0){
            list_add(table_list, ctext);
        }
    }
    if (rc != SQLITE_DONE) {
        printf("error: ", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}
