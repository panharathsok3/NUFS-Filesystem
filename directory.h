// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME 48

#include "slist.h"
#include "pages.h"
#include "inode.h"

/*
 * Represents a directory structure, holding its name, inode number, and flags.
 */
typedef struct dirent {
    char name[DIR_NAME];
    int  inum;
    char _reserved[12];
} dirent;

// Initializes the directory
void directory_init();
// Looks up the given directory name
int directory_lookup(inode* dd, const char* name);
// Looks up the given tree path
int tree_lookup(const char* path);
// Places a directory
int directory_put(inode* dd, const char* name, int inum);
// Deletes a directory
int directory_delete(inode* dd, const char* name);
// Lists out all the directories
slist* directory_list(const char* path);
// Prints the given directory
void print_directory(inode* dd);
// Returns the parent of the directory
int directory_get_super(const char* path);
// Returns the name of the directory
char* directory_get_name(const char* path);

#endif

