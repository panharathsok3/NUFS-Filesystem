#include "inode.h"
#include "pages.h"
#include "bitmap.h"
#include <stdint.h>
#include "util.h"



// Max number of nodes
int MAX_NODE = 4096 / sizeof(inode);

/*
 * Returns the inode with the particular inode number.
 */
inode* get_inode(int inum) {
    return (inode*) (((uint8_t*) pages_get_page(0)) + 64) + (inum * sizeof(inode));
}

/*
 * Creates an inode at the given inode number.
 */
int alloc_inode(int inum) {
    void* inbm = get_inode_bitmap();
    // Setting the bitmap position to 1
    for (int i = 0; i < MAX_NODE; i++) {
        if (!bitmap_get(inbm, i)) {
            bitmap_put(inbm, i, 1);
            return i;
        }
    }
    return -1;
}

/*
 * Frees the inode at the given inode number.
 */
void free_inode(int inum) {
    inode* node = get_inode(inum);
    void* inbm = get_inode_bitmap();

    if (node->refs > 1) {
        node->refs = node->refs - 1;
        return;
    } 
    else {
        free_page(node->ptrs[0]);
        free_page(node->ptrs[1]);
        memset(node, 0, sizeof(inode));
        bitmap_put(inbm, inum, 0);
    }
}

/*
 * Expands the size of the inode.
 */
int grow_inode(inode* node, int size) {
	int new_pages = bytes_to_pages(size);
	
	if (new_pages > 2){
		new_pages = new_pages - 2;
		if (node->iptr == 0){
			node->iptr =  alloc_page();
		}
		int* indirect = (int*) pages_get_page(node->iptr);
		
		for (int i = 0; i < new_pages; i++) {
			if (!indirect[i]){
				indirect[i] = alloc_page();
			}
		}
	}
    //printf("growing inode: from size %d, to size: %d---------", node->size, size);
    node->size = size;
	return node->size;
}

/*
 * Returns the page number of the given inode.
 */
int inode_get_pnum(inode* node, int fpn) {
    return fpn < 2 ? node->ptrs[fpn] : ((int*)pages_get_page(node->iptr))[fpn - 2];
}


//TODO
//this is needed for last test for deleting
int shrink_inode(inode* node, int size) {
    
    //printf("shrinking inode: from size %d, to size: %d---------------------", node->size, node->size - size);

    node->size = node->size - size;
    return 0;
}