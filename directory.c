#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>


#include "directory.h"
#include "slist.h"
#include "pages.h"
#include "inode.h"
#include "util.h"
#include "bitmap.h"

/*
 * Represents functions for a directory.
 */

// Current root
int rooti = 0;
// Maximum number of entries
int MAX_ENTR = 4096 / sizeof(dirent);

/*
 * Initializes the directory.
 */
void directory_init() {
    // Inode bitmap
	void* ibm =  get_inode_bitmap();	
	// Creates the root inode
	rooti = alloc_inode();
	inode* root = get_inode(rooti);

	root->refs = 1;

    // Root is the inode root
	root->mode = 040755;
	root->size = 0;
	root->ptrs[0] = alloc_page();
	root->ptrs[1] = alloc_page();
	root->iptr = 0;
    root->time = time(0);

    // Sets the inode bitmap first spot to 1, because root was created
	bitmap_put(ibm, 0, 1);

    // Root points to itself
	directory_put(root, ".", rooti);
}


/*
 * Looks up the given directory inode and name.
 * Takes in an inode and the string of the directory name.
 * Returns the inode number of the specified directory.
 */
int directory_lookup(inode* dd, const char* name) {
    dirent* entries = (dirent*) pages_get_page(dd->ptrs[0]);
    for (int i = 0; i < MAX_ENTR; i++, entries = (void*)entries + sizeof (dirent)) {
        if(streq(entries->name, name)) {
			return entries->inum;
		}
    }
    return -ENOENT;
}

/*
 * Looks up the root of the tree.
 * Takes in the string for the path.
 * Returns the inode corresponding inode number.
 */
int tree_lookup(const char* path) {

    int result = 0;
    inode* root = get_inode(rooti);

    slist* list = s_split(path, '/');
	list = list->next;

    if (streq(path, "/")) {
        s_free(list);
        return rooti;
    } else {
		char* filename = directory_get_name(path);
		result = directory_lookup(get_inode(directory_get_super(path)),filename);
		s_free(list);
		return result;
    }
}

/*
 * Places an inode into the directory with the given inode number and name.
 * Takes in an inode, the string name, and the integer inode number.
 * Returns 0 upon success, 1 otherwise.
 */
int directory_put(inode* dd, const char* name, int inum) {
    dirent* entries = (dirent*) pages_get_page(dd->ptrs[0]);
    for (int i = 0; i < MAX_ENTR; i++) {
		if(entries[i].inum == 0) {
            entries[i].inum = inum;
			strcpy(entries[i].name, name);
            dd->size += sizeof (dirent);
            dd->time = time(0);
			return 0;
		}
	}
	return -1;
}

/*
 * Deletes the given directory with the given name.
 */
int directory_delete(inode* dd, const char* name) {
    int rv = 0;
    printf(" + directory_delete(%s)\n", name);
    int rm = 0;
    dirent* entries = (dirent*) pages_get_page(dd->ptrs[0]);
    for (int i = 0; i < MAX_ENTR; i++) {
		if(!(strcmp(entries[i].name, name))) {
            rm = i;
            goto delete_found;
		}
	}
    delete_found:
    dd->time = time(0);
    memcpy(&entries[rm], &entries[rm + 1], 4096 - ((rm + 1) * sizeof(dirent)));
    return rv;
}

/*
 * Creates a list of all the directories in a particular path.
 */
slist* directory_list(const char* path) {

    int index = get_inode(tree_lookup(path))->ptrs[0];
    dirent* entries = (dirent*) pages_get_page(index);

    slist* result = 0;
    for (int i = 0; i < MAX_ENTR && strcmp(entries[i].name, ""); i++) {
        result = s_cons(entries[i].name, result);
    }
    return result;

}

/*
 * Retrieves the parent directory
 */
int directory_get_super(const char* path) {

    int result = 0;
	slist* list = s_split(path, '/');
	list = list->next;
	
    // in root directory
	if (!list->next) {
		return 0;
	}

    inode* parent = get_inode(rooti);
    for (; list->next && list->next->next && result > 0; list = list->next) {
        result = directory_lookup(parent, list->data);
        parent = get_inode(result);
    }
    result = directory_lookup(parent, list->data);
    s_free(list);

    return result;

}

/*
 * Returns the data of the given path's furthest node.
 */
char* directory_get_name(const char* path) {
    slist* list = s_split(path, '/');
    while(list->next) {
        list = list->next;
    }
    return list->data;
}

