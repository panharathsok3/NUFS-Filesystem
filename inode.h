// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pages.h"

/*
 * Represents an inode that points to a directory.
 */
typedef struct inode {
    int refs; // reference count
    int32_t  mode; // permission & type
    int size; // bytes
    int ptrs[2]; // direct pointers
    int iptr; // single indirect pointer
    time_t time;

    
} inode;

// Prints out attributes of inode
void print_inode(inode* node);
// Returns the inode with the specified inode number
inode* get_inode(int inum);
// Creates an inode
int alloc_inode();
// Frees the inode of the given number
void free_inode(int inum);
// Increases the inode's size
int grow_inode(inode* node, int size);
// Shrinks the inode
int shrink_inode(inode* node, int size);
// Returns the number of the inode
int inode_get_pnum(inode* node, int fpn);

#endif
