#include "db_ops.h"
#include "sqlite3.h"

#include "stdio.h"
#include "dirent.h"
#include "sys/stat.h"
#include "sys/types.h"

#define MEDIA_FOLDER "media";
#define STMT_MAX 500;


typedef struct cv_pairs {
    char table[30];
    char column[10][100];
    char value[10][100];
};




void *db_open2(const char *db_path) {
    sqlite3 *db = NULL;
    if (sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                        SQLITE_OPEN_FULLMUTEX, NULL) == SQLITE_OK) {
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS media_1(ID INTEGER PRIMARY KEY AUTOINCREMENT, video_title TEXT, description TEXT, date_added DATE DEFAULT (datetime('now','localtime')));", 0, 0, 0);
    }
    db_update(&db);
    return db;
}


void db_close2(void **db_handle) {
  if (db_handle != NULL && *db_handle != NULL) {
    sqlite3_close(*db_handle);
    *db_handle = NULL;
  }
}


void db_get_by_key(void **db, const char *key){
} 


void db_get_by_tag(void **db, const char *tag){
}


void db_add(void **db, const char *table_name, FILE *dp){
    char stmt[STMT_MAX] = "INSERT INTO \0";
    strcat(stmt, table_name);
    strcat(stmt, " (" 
    strcat(column_names);
    strcat(stmt, ") VALUES ("); 
    strcat(stmt, video_title); 
    strcat(stmt, ", 'test description');");
    fprintf(stdout, "%s\n", stmt);
    sqlite3_exec(db, stmt, 0, 0, 0);
}


void parse_file(FILE *fp, struct cv_pair **cv){
    //video title
    char t[100];
    strcpy(t, fp->d_name);
    int i;
    for (i = strlen(t - 1); i >= 0; i--){
        if (t[i] == '.'){
            t[i] = '\0';
            break;
        }
        t[i] = '\0';
    }
    strcpy(cv.column[0], "video_title");
    strcpy(cv.value[0], t);
    //description
}





void db_update(void **db_handle){
    //loop through media folder
    struct dirent *dp;
    DIR *dfd;

    const char *dir = MEDIA_FOLDER;
    if ((dfd = opendir(dir)) == NULL){
        fprintf(stderr, "Could not open %s folder\n Exiting...", dir);
        exit(0);
    }
    //each file put entry in database
    while ((dp = readdir(dfd)) != NULL){
        if (dp->d_name[0] != '.'){
            fprintf(stdout, "%s\n", dp->d_name);
            db_add(db, "media_1", dp);
        }
    }
}
