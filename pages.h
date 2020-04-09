// based on cs3650 starter code

#ifndef PAGES_H
#define PAGES_H

#include <stdio.h>

/*
 * Represents a page in the directory.
 */

// Initializes a page with the given path
void pages_init(const char* path);
// Frees all the pages
void pages_free();
// Returns the page with the given page number
void* pages_get_page(int pnum);
// Gets the page bitmap
void* get_pages_bitmap();
// Gets the inode bitmap
void* get_inode_bitmap();
// Allocates a page
int alloc_page();
// Frees the given page
void free_page(int pnum);

#endif
