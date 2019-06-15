#include "database.h"
#include "sqlite3.h"
#include "queue.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "sys/stat.h"
#include "sys/types.h"


#define CREATE_STMT_0 "CREATE TABLE IF NOT EXISTS "
#define CREATE_STMT_1 " (ID INTEGER PRIMARY KEY AUTOINCREMENT, "\
                    "location TEXT, title TEXT, description TEXT, "\
                    "category TEXT, date_added DATE DEFAULT "\
                    "(datetime('now','localtime')));"


void db_add_vids_from_folder(sqlite3 *db, char *foldername, char *table);
void db_add_by_sum(sqlite3 *db, char *table, struct vid_sum *sum);
int db_exists(sqlite3 *db, char *table, struct vid_sum *sum);



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
    char foldername[NAME_MAX*2];
    char create_sql[STMT_MAX];
    sqlite3_stmt *stmt;
    int rc;

    if ((dfd = opendir(MEDIA_FOLDER)) == NULL){
        fprintf(stderr, "Could not open %s folder\n Exiting...\n", MEDIA_FOLDER);
        exit(0);
    }

    if (sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                        SQLITE_OPEN_FULLMUTEX, NULL) == SQLITE_OK) {
        while ((dp = readdir(dfd)) != NULL){
            if (dp->d_name[0] == '.')
                continue;
            sprintf(foldername, "%s/%s", MEDIA_FOLDER, dp->d_name);
            if (stat(foldername, &stbuf) == -1)
                continue;
            if ((stbuf.st_mode & S_IFMT) == S_IFDIR){
		memset(create_sql, 0, STMT_MAX);
		strcpy(create_sql, CREATE_STMT_0);
		strcat(create_sql, dp->d_name);
		strcat(create_sql, CREATE_STMT_1);
		printf("%s\n", create_sql);
		rc = sqlite3_prepare_v2(db, create_sql, -1, &stmt, NULL);
		if (rc != SQLITE_OK){
		    fprintf(stderr, "Failed to prepare statement\n Exiting...\n");
        	    exit(0);
	    	}
		rc = sqlite3_step(stmt);
		if (rc == SQLITE_ERROR){
		    fprintf(stderr, "Failed to execute statement\n Exiting...\n");
        	    exit(0);
	    	}
		sqlite3_finalize(stmt);
		db_add_vids_from_folder(db, foldername, dp->d_name);
            }
        }
    }
    return db;
}


void db_add_vids_from_folder(sqlite3 *db, char *foldername, char *table){
    DIR *dfd;    	
    struct dirent *dp;
    struct vid_sum sum;
    dfd = opendir(foldername);
    while ((dp = readdir(dfd)) != NULL){
        if (dp->d_name[0] == '.')
       	    continue;
	memset(sum.location, 0, 1000);
	strcpy(sum.location, foldername);
	strcat(sum.location, "/");
	strcat(sum.location, dp->d_name);
	memset(sum.title, 0, 1000);
	strcpy(sum.title, strip_filetype(dp->d_name));
	memset(sum.description, 0, 1000);
	strcpy(sum.description, "EXAMPLE DESCRIPTION");
	memset(sum.category, 0, 1000);
	strcpy(sum.category, "DEFAULT CATEGORY");
	if (!db_exists(db, table, &sum)){
	    db_add_by_sum(db, table, &sum);
	}
    }
}


void db_add_by_sum(sqlite3 *db, char *table, struct vid_sum *sum){
    char insert_sql[STMT_MAX];
    sqlite3_stmt *stmt;
    int rc;
    strcpy(insert_sql, "INSERT INTO ");
    strcat(insert_sql, table); 
    strcat(insert_sql, " ( location, title, description, category"); 
    strcat(insert_sql, ") VALUES (\""); 
    strcat(insert_sql, sum->location); 
    strcat(insert_sql, "\", \""); 
    strcat(insert_sql, sum->title); 
    strcat(insert_sql, "\", \""); 
    strcat(insert_sql, sum->description); 
    strcat(insert_sql, "\", \""); 
    strcat(insert_sql, sum->category); 
    strcat(insert_sql, "\");");
    printf("%s\n", insert_sql);
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Failed to prepare statement\n Exiting...\n");
        exit(0);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ERROR){
        fprintf(stderr, "Failed to execute statement\n Exiting...\n");
        exit(0);
    }
    sqlite3_finalize(stmt);
		
}


void db_close(void **db_handle) {
  if (db_handle != NULL && *db_handle != NULL) {
    sqlite3_close(*db_handle);
    *db_handle = NULL;
  }
}


int db_exists(sqlite3 *db, char *table, struct vid_sum *sum){
    sqlite3_stmt *stmt;
    char select_sql[STMT_MAX];
    int rc;
    memset(select_sql, 0, STMT_MAX);
    strcpy(select_sql, "SELECT * FROM ");
    strcat(select_sql, table);
    strcat(select_sql, " WHERE ");
    if (sum->location != NULL){
	strcat(select_sql, "location=\"");
	strcat(select_sql, sum->location);
	strcat(select_sql, "\";");
    } else {
	return 0;
    }
    printf("%s\n", select_sql);
    rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Failed to prepare statement\n Exiting...\n");
        exit(0);
    }
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_ROW){
	return 1;
    } else if (rc == SQLITE_ERROR){
        fprintf(stderr, "Failed to execute statement\n Exiting...\n");
        exit(0);
    } else {
	return 0;
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


void db_get_tables(void *db, struct list *table_list){
    char select_sql[STMT_MAX] = "SELECT name FROM sqlite_master WHERE type='table'";
    sqlite3_stmt *stmt;
    int rc ;
    rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, NULL);
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


void db_get_all_sum(sqlite3 *db, char *table, struct vid_sum **sum, int *n){
    char select_sql[STMT_MAX];
    char t[1000];
    sqlite3_stmt *stmt;
    int rc, i;
    struct vid_sum *sumv;
    void *temp;
    memset(select_sql, 0, STMT_MAX);
    strcpy(select_sql, "SELECT * FROM ");
    strcat(select_sql, table);
    strcat(select_sql, " ORDER BY date_added DESC;");

    printf("%s\n", select_sql);
    rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK){
        fprintf(stderr, "Failed to prepare statement\n Exiting...\n");
        exit(0);
    }
    sumv = malloc(sizeof(struct vid_sum));
    i = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
	temp = realloc(sumv, sizeof(struct vid_sum) * (i + 1));
	if (temp != NULL)
	    sumv = temp;
	memset(sumv[i].location, 0, 1000);
        strcpy(sumv[i].location, sqlite3_column_text(stmt, 1));
	memset(sumv[i].title, 0, 1000);
        strcpy(sumv[i].title, sqlite3_column_text(stmt, 2));
	memset(sumv[i].description, 0, 1000);
        strcpy(sumv[i].description, sqlite3_column_text(stmt, 3));
	memset(sumv[i].category, 0, 1000);
        strcpy(sumv[i].category, sqlite3_column_text(stmt, 4));
	i++;
    }
    n = i;
    *sum = sumv;
    if (rc != SQLITE_DONE) {
        printf("error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
	
}
