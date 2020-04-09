// based on cs3650 starter code

#ifndef SLIST_H
#define SLIST_H

/*
 * Represents a list that holds a string, number of references, and the next linked list.
 */
typedef struct slist {
    char* data;
    int   refs;
    struct slist* next;
} slist;

// Adds a string to the linked list
slist* s_cons(const char* text, slist* rest);
// Frees the linked list
void s_free(slist* xs);
// Splits the linked list based on the given delimiter
slist* s_split(const char* text, char delim);

#endif

