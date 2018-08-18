#include "template.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#define OPEN_TOKEN "{%"
#define CLOSE_TOKEN "%}"




//replace all occurances of s1 with s2 in html
static void replace(char **html, char *s1, char *s2){
    char *str_s, *str_f, *str_res, *p, *t, *t2;
    size_t res_len;
    p = *html;
    str_s = strstr(p, OPEN_TOKEN);
    str_f = strstr(p, CLOSE_TOKEN);
    fprintf(stdout, "%s:%s\n", s1, s2);
    while(str_s != NULL && str_f != NULL){
        if (str_s > str_f){
            return;
        }
        str_res = malloc((str_f - str_s) * sizeof(char));
        strncpy(str_res, str_s + strlen(OPEN_TOKEN), str_f - str_s - strlen(OPEN_TOKEN));
        str_res[str_f - str_s - strlen(OPEN_TOKEN)] = '\0';
        if (strcmp(str_res, s1) == 0){
            str_f += strlen(CLOSE_TOKEN);
            t = malloc(strlen(str_f) * sizeof(char));
            strcpy(t, str_f);
            *str_s = '\0';
            t2 = realloc(*html, strlen(*html) + strlen(s2) + strlen(t));
            if (t2 != NULL)
                *html = t2;
            strcat(*html, s2);
            strcat(*html, t);
            free(t);
            p = str_s;
        } else {
            p = str_f + strlen(CLOSE_TOKEN);
        }
        str_s = strstr(p, OPEN_TOKEN);
        str_f = strstr(p, CLOSE_TOKEN);
        free(str_res);
    }
}


static int read_template(const char *loc, char **templ){
    FILE *fp;
    long fsize;
    size_t rsize;
    if ((fp = fopen(loc, "r")) != 0){
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        rewind(fp);
        *templ = malloc(fsize * sizeof(char));
        rsize = fread(*templ, 1, fsize, fp);
        if (rsize != fsize){
            fprintf(stderr, "Read error in template\n");
            return 0;
        }
        fclose(fp);
        return 1;
    }
    return 0;
}



void gen_browse_html(const char *path, char **html){
    FILE *fp;
    long fsize;
    size_t rsize;
    if ((fp = fopen(VIDEO_TEMPLATE, "r")) != 0){
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        rewind(fp);
        *html = malloc(fsize * sizeof(char));
        rsize = fread(*html, 1, fsize, fp);
        if (rsize != fsize){
            fprintf(stderr, "Read error in template\n");
            return;
        }
        fclose(fp);
        replace(html, "VIDEO_LOCATION", path);
    }
}


void gen_video_html(const char *fpath, char **html){
    FILE *fp;
    long fsize;
    size_t rsize;
    char *vdiv = NULL;
    if ((fp = fopen(VIDEODIV_TEMPLATE, "r")) != 0){
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        rewind(fp);
        vdiv = malloc(fsize * sizeof(char));
        rsize = fread(vdiv, 1, fsize, fp);
        if (rsize != fsize){
            fprintf(stderr, "Read error in template\n");
            return;
        }
        fclose(fp);
    }
    if ((fp = fopen(BROWSE_TEMPLATE, "r")) != 0 && vdiv != NULL){
        //get filesize
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        rewind(fp);
        *html = malloc(fsize * sizeof(char));
        rsize = fread(*html, 1, fsize, fp);
        if (rsize != fsize){
            fprintf(stderr, "Read error in template\n");
            return;
        }
        fclose(fp);
        replace(html, "VIDEO_DIV", vdiv);
        replace(html, "VIDEO_ID", "420");
        replace(html, "VIDEO_THUMB", "thumb/example.png");
        replace(html, "VIDEO_TITLE", "REOL");
        replace(html, "VIDEO_DESC", "test description");
    }
}


//TODO FUCKING ENTER IN THE STUPID LIST OUTPUT WTF
void gen_home_html(char **html, struct str_list *list){
    char *div, *t, *res;
    size_t i;
    char *next = OPEN_TOKEN "CAT_DIV" CLOSE_TOKEN;
    if (!read_template(HOME_TEMPLATE, html))
        return;
    if (!read_template(CATDIV_TEMPLATE, &div))
        return;
    for (i = 0; i < list->count; i++){
        replace(html, "CAT_DIV", div);
        list_get(list, i, &t);
        replace(html, "CAT_TITLE", t);
        replace(html, "CAT_APPEND", next);
    }
    replace(html, "CAT_DIV", "");
}



