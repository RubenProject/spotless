#include "template.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#define OPEN_TOKEN "{%"
#define CLOSE_TOKEN "%}"




//replace all occurances of s1 with s2 in html
static void replace(char **html, char *s1, char *s2){
    char *str_s, *str_f, *str_res, *p, *t, *t2;
    p = *html;
    str_s = strstr(p, OPEN_TOKEN);
    str_f = strstr(p, CLOSE_TOKEN);
    //fprintf(stdout, "%s:%s\n", s1, s2);
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
            t2 = realloc(*html, strlen(*html) + strlen(s2) + strlen(t) + 1);
            if (t2 != NULL)
                *html = t2;
            else 
                return;
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



void gen_browse_html(char **html){
    char *div;
    if (!read_template(BROWSE_TEMPLATE, html)){
        fprintf(stderr, "Could not open browse template\n");
        return;
    }
    if (!read_template(VIDEODIV_TEMPLATE, &div)){
        fprintf(stderr, "Could not open videodiv template\n");
        return;
    }
}


void gen_video_html(char **html){
    if (!read_template(BROWSE_TEMPLATE, html)){
        fprintf(stderr, "Could not open video template\n");
        return;
    }
    replace(html, "VIDEO_TITLE", "REOL");
    replace(html, "VIDEO_LOC", "REOL");
    replace(html, "VIDEO_DESC", "test description");
}


void gen_home_html(char **html, struct list *list){
    size_t i;
    char *div, *t;
    char *next = OPEN_TOKEN "CAT_DIV" CLOSE_TOKEN;
    if (!read_template(HOME_TEMPLATE, html)){
        fprintf(stderr, "Could not open home template\n");
        return;
    }
    if (!read_template(CATDIV_TEMPLATE, &div)){
        fprintf(stderr, "Could not open catdiv template\n");
        return;
    }
    for (i = 0; (t = list_get(list, i)) != NULL; i++){
        replace(html, "CAT_DIV", div);
        replace(html, "CAT_TITLE", t);
        replace(html, "CAT_APPEND", next);
    }
    replace(html, "CAT_DIV", "");
    free(div);
    free(t);
}



