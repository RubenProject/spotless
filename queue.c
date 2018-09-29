/*
 * Copyright (C) 2014 Noel Kuntze <noel@familie-kuntze.de>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct element {
	struct element *next;
	char *content;
};

struct list {
	struct element *first;
	struct element *last;
};

struct list *list_init (void) {
	struct list *list = malloc(sizeof(struct list));
	list->first = NULL;
	list->last = NULL;

	return list;
}

int list_push (struct list *list, char *string) {
	/* Check for users doing stupid things  */
	if (list == NULL) {
		fprintf(stderr, "The first argument to push needs to be an initialized list!\n");
		return 1;
	}

	/* allocate memory and check return value */
	struct element *element = malloc(sizeof(struct element));

	if (element == NULL) {
		fprintf (stderr, "The program could not allocate enough memory for a new element.\n");
		return 1;
	}

	element->next = NULL;

	/* copy content to the new element */
	if (string != NULL) {
		size_t len = strlen(string)+1;
		element->content = malloc(len);
		if (element->content == NULL) {
			fprintf(stderr, "The program could not allocate enough memoryr for the content of the new element.\n");
		}
		memcpy(element->content, string, len);	
	} else {
		element->content = NULL;
	}
	
	/* change the pointers to accomodate the new element */	
        /* Do other suff, if this is the first element in a new list */
	if (list->first == NULL)
                list->first = element;
	if (list->last != NULL) {
		list->last->next = element;
		list->last = element;
	} else {
		/* Do stuff, if the new element isn't the first one in a new list */
		list->last = element;
	}
	return 0;
}
/* This function returns and removes the last object in the list */
char *list_pop (struct list *list) {

	if (list == NULL) {
        fprintf(stderr, "The first argument to pop needs to be an initialized list!\n");
        return NULL;
	}

	if (list->first == NULL) {
		return NULL;
	}

	struct element *lastButOne = NULL;

	char *string = NULL;

	if (list->last == list->first) {
		string = list->last->content;
		free (list->last);
		list->last = NULL;
		list->first = NULL;
		return string;
	}
	lastButOne = list->first;
	while (lastButOne->next != list->last) {
		lastButOne = lastButOne->next;
	}
	string = lastButOne->next->content;
	free(lastButOne->next);
	list->last = lastButOne;
	lastButOne->next = NULL;
	return string;
}


char *list_get(struct list *list, size_t i) {
	if (list == NULL) {
        fprintf(stderr, "The first argument to pop needs to be an initialized list!\n");
        return NULL;
	}

	if (list->first == NULL) {
		return NULL;
	}

    struct element *p = NULL;

    p = list->first;
    while (i > 0){
        if (p == list->last || p == NULL)
            return NULL;
        p = p->next;
        i--;
    }
    return p->content;
}


int list_free (struct list *list) {

	if(list == NULL) {
		fprintf(stderr, "The first argument has to be an initialized list!\n");
		return 1;
	}

	if (list->first == NULL) {
		free(list);
		return 0;
	}

	if (list->first->next == NULL) {
		free(list->first->content);
		free(list->first);
		free(list);
		return 0;
	}

	char *test;
	do {
		test=list_pop(list);
		if (test != NULL)
			free(test);

	} while (test != NULL);
	
	free(list);

	return 0;
}
