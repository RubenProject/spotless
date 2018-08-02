#include "db_ops.h"
#include "sqlite3.h"

#include "stdio.h"
#include "dirent.h"
#include "sys/stat.h"
#include "sys/types.h"

#define MEDIA_FOLDER "media";


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
    struct stat stbuff;
    char stmt[500];
    strcpy(stmt, "INSERT INTO media_1 (video_title, description, date_added) VALUES ('test title', 'test description');");
    while ((dp = readdir(dfd)) != NULL){
        if (dp->d_name[0] != '.'){
            fprintf(stdout, "%s\n", dp->d_name);
            sqlite3_exec(db_handle, stmt, 0, 0, 0);

        }
    }

    
}
