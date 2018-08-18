#include "list.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"



struct str_list *list_create(){
    struct str_list *list;
    list = malloc(sizeof(struct str_list));
    list->p = NULL;
    list->size = 0;
    list->count = 0;
    return list;
}


int list_add(struct str_list **list, char *str){
    if (*list == NULL){
        return 0;
    }
    void *temp = realloc((*list)->p, (*list)->size + strlen(str) + 2);
    if (temp == NULL)
        return 0;
    (*list)->p = temp;
    memcpy((*list)->p + (*list)->size, str, strlen(str));
    (*list)->size += strlen(str) + 1;
    (*list)->p[(*list)->size - 1] = SEP_TOKEN;
    (*list)->p[(*list)->size] = '\0';
    (*list)->count++;
    return 1;
}


int list_get(struct str_list *list, size_t elem, char **str){
    if (list->p == NULL)
        return 0;
    if (elem >= list->count || list->count == 0){
        return 0;
    }
    char *p, *q, *r;
    size_t i;
    p = list->p;
    for (i = 0; i < elem; i++){
        while (*p != SEP_TOKEN){
            p++;
        }
        p++;
    }
    if (elem == list->count - 1){
        r = malloc(strlen(p));
        strcpy(r, p);
    } else {
        q = p;
        while (*q != SEP_TOKEN){
            q++;
        }
        r = malloc(q - p);
        strncpy(r, p, q - p);
        r[q - p] = '\0';
    }
    *str = r;
    return 1;
}


void list_free(struct str_list **list){
    free((*list)->p);
    free(*list);
    list = NULL;
}

