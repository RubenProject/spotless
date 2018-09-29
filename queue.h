#ifndef VAR_QUEUE_H
#define VAR_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct element {
	struct element *next;
	char *content;
};

struct list {
	struct element *first;
	struct element *last;
};

struct list *list_init (void);

int list_push (struct list *list, char *string);

char *list_get(struct list *list, size_t i);

/* This function returns and removes the last object in the list */
char *list_pop (struct list *list);

int list_free (struct list *list);


#endif
