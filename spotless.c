/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "mongoose/mongoose.h"
#include "database.h"
#include "template.h"
#include "list.h"

#include "dirent.h"
#include "sys/types.h"

#define MEDIA_FOLDER "web_root/media"
#define NAME_MAX 100

static const char *s_http_port = "8004";
static struct mg_serve_http_opts s_http_server_opts;
static int s_sig_num = 0;
static void *s_db_handle = NULL;
static const char *s_db_path = "spotless.db";


struct file_writer_data {
    char filename[NAME_MAX];
    void *file_data;
    size_t file_size;
    char tablename[NAME_MAX];
    struct sql_data *entry;
    struct buffer_data *buff;
};


struct buffer_data {
    void *p;
    size_t size;
};


static void signal_handler(int sig_num) {
    signal(sig_num, signal_handler);
    s_sig_num = sig_num;
}


static void copy_buffer(struct file_writer_data *data, struct mg_http_multipart_part *mp) {
    if (strcmp(mp->var_name, "file") == 0){
        strcpy(data->tablename, "general");
        strcpy(data->filename, mp->file_name);

        data->file_data = malloc(data->buff->size);
        data->file_size = data->buff->size;
        memcpy(data->file_data, data->buff->p, data->buff->size);

        strcpy(data->entry->column[data->entry->count], "filename");
        strcpy(data->entry->value[data->entry->count], mp->file_name);
        data->entry->count++;
    } else {
        strcpy(data->entry->column[data->entry->count], mp->var_name);
        memcpy(data->entry->value[data->entry->count], data->buff->p, data->buff->size);
        data->entry->value[data->entry->count][data->buff->size] = '\0';
        data->entry->count++;
    }
}


static void write_file (struct file_writer_data *data) {
    struct sql_data *p = malloc(sizeof(struct sql_data));
    int rc;
    FILE *fp;
    char path[NAME_MAX];
    if (!db_get_by_fname(s_db_handle, data->tablename, data->filename, p)){
        db_add(s_db_handle, data->tablename, data->entry);
        sprintf(path, "%s/%s/%s", MEDIA_FOLDER, data->tablename, data->filename);
        if ((fp = fopen(path, "wb")) != 0){
            fprintf(stdout, "writing file: %s\n", path);
            fwrite(data->file_data, 1, data->file_size, fp);
            fclose(fp);
        }
    }
}
 

static void handle_upload(struct mg_connection *nc, int ev, void *ev_data) {
    struct file_writer_data *data = (struct file_writer_data *) nc->user_data;
    struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) ev_data;

    switch (ev) {
        case MG_EV_HTTP_PART_BEGIN: {
            if (data == NULL) {
                data = calloc(1, sizeof(struct file_writer_data));
            }
            if (data->entry == NULL){
                data->entry = calloc(1, sizeof(struct sql_data));
                data->entry->count = 0;
            }
            if (data->buff == NULL) {
                data->buff = calloc(1, sizeof(struct buffer_data));
                data->buff->p = NULL;
                data->buff->size = 0;
                nc->user_data = (void *) data;
            }
          break;
        }
        case MG_EV_HTTP_PART_DATA: {
            void *temp = realloc(data->buff->p, data->buff->size + mp->data.len);
            if (temp == NULL){
                mg_printf(nc, "%s",
                  "HTTP/1.1 500 Failed to write to a file\r\n"
                  "Content-Length: 0\r\n\r\n");
                nc->flags |= MG_F_SEND_AND_CLOSE;
                return;
            }
            data->buff->p = temp;
            memcpy(data->buff->p + data->buff->size, mp->data.p, mp->data.len);
            data->buff->size += mp->data.len;
            break;
        }
        case MG_EV_HTTP_PART_END: {
            mg_printf(nc,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n"
                "Written %ld of POST data to a temp file\n\n",
                data->buff->size);
            nc->flags |= MG_F_SEND_AND_CLOSE;

            //copy content of buffer to correct field
            copy_buffer(data, mp);
            //free up temporary buffer
            free(data->buff->p);
            free(data->buff);
            data->buff = NULL;
            //write to database when all fields are full
            if (data->file_data != NULL && data->entry->count == COLUMN_MAX){
                fprintf(stdout, "received file: %s\n", data->filename);
                write_file(data);
                free(data);
                nc->user_data = NULL;
            }
            break;
        }
    }
}


static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    struct http_message *hm = (struct http_message *) ev_data;
    struct str_list *table_list;
    char *html;
    switch (ev) {
        case MG_EV_HTTP_REQUEST:
            table_list = list_create();
            db_get_tables(s_db_handle, &table_list);
            gen_home_html(&html, table_list);
            mg_send_head(nc, 200, strlen(html), "Content-Type: text/html");
            mg_printf(nc, "%.*s", (int)strlen(html), html);
            list_free(&table_list);
            //mg_serve_http(nc, hm, s_http_server_opts);
            break;
            //serve the homepage
            //if (strncmp(hm->uri.p, "/", hm->uri.len) == 0){
                //struct str_list *table_list;
                //table_list = list_create();
                //db_get_tables(s_db_handle, &table_list);
                //list_free(&table_list);
            //}
            // serve specific video
            //} else if (strncmp(hm->uri.p, "/video", hm->uri.len) == 0){
             //   fprintf(stdout, "unimplemented\n");
            // show all videos in a category by date added
            //} else {
        default:
            break;
    }
}


int main(int argc, char *argv[]) {
    struct mg_mgr mgr;
    struct mg_connection *nc;
    int i;

    /* Open listening socket */
    mg_mgr_init(&mgr, NULL);
    nc = mg_bind(&mgr, s_http_port, ev_handler);
    if (nc == NULL){
        printf("Could not start server on port %s\n", s_http_port);
        exit(0);
    }

    mg_register_http_endpoint(nc, "/upload", handle_upload MG_UD_ARG(NULL));
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = "web_root";

    /* Parse command line arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-D") == 0) {
            mgr.hexdump_file = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0) {
            s_db_path = argv[++i];
        } else if (strcmp(argv[i], "-r") == 0) {
        s_http_server_opts.document_root = argv[++i];
        }
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Open database */
    if ((s_db_handle = db_open(s_db_path)) == NULL) {
        fprintf(stderr, "Cannot open DB [%s]\n", s_db_path);
        exit(EXIT_FAILURE);
    }

    /* Run event loop until signal is received */
    printf("Starting RESTful server on port %s\n", s_http_port);
    while (s_sig_num == 0) {
        mg_mgr_poll(&mgr, 1000);
    }

    /* Cleanup */
    mg_mgr_free(&mgr);
    db_close(&s_db_handle);
    
    printf("Exiting on signal %d\n", s_sig_num);

    return 0;
}
