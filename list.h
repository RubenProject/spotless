#ifndef LIST_H
#define LIST_H

#define SEP_TOKEN '\n'


#include "stdlib.h"

struct str_list {
    char *p;
    size_t size;
    int count;
};


struct str_list *list_create();
int list_add(struct str_list **list, char *str);
int list_get(struct str_list *list, size_t elem, char **str);
void list_free(struct str_list **list);






#endif
