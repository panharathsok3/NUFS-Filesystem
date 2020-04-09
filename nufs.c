// based on cs3650 starter code

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
//#include <dirent.h>
#include <bsd/string.h>
#include <assert.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>
#include "inode.h"
#include "util.h"
#include "bitmap.h"
#include "pages.h"
#include "slist.h"
#include "directory.h"


int PAGE_SIZE = 4096;

// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask)
{
    int rv = 0;

    if (tree_lookup(path) < 0) {
        rv = -ENOENT;
    }
    printf("access(%s, %04o) -> %d\n", path, mask, rv);
    return rv;
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
{
	int inum  = tree_lookup(path);
	int rv = 0;
	if (inum == -ENOENT){
		return rv = -ENOENT;
	}
	else {
	inode* node = get_inode(inum);
	st->st_mode = node->mode;
	st->st_size = node->size;
    st->st_nlink = node->refs;
    st->st_mtime = node->time;
	}

    printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode, st->st_size);
	return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    struct stat st;


    for(slist* list = directory_list(path); list; list = list->next) {
        filler(buf, list->data, &st, 0);
    }

    printf("readdir(%s) -> %d\n", path, 0);
    return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int rv = -1;

    int inum = alloc_inode();
	inode* newnode = get_inode(inum);
    int dirNum = directory_get_super(path);
	inode* node = get_inode(dirNum);

    newnode->ptrs[0] = alloc_page();
    newnode->ptrs[1] = alloc_page();
    newnode->refs = 1;
    newnode->mode = mode;
    newnode->iptr = 0;
    newnode->size = 0;
    newnode->time = time(0);

    rv = directory_put(get_inode(directory_get_super(path)), directory_get_name(path), inum);




    printf("mknod(%s, %04o) -> %d\n", path, mode, rv);



    return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
    int rv = nufs_mknod(path, mode | 040000, 0);
    printf("mkdir(%s) -> %d\n", path, rv);
    return rv;
}


int remove(const char* path) {
    int rv = -1;

    int inum = tree_lookup(path);
    free_inode(inum);

    rv =  directory_delete(get_inode(directory_get_super(path)), directory_get_name(path));
    return rv;
}


int
nufs_unlink(const char *path)
{


    int rv = remove(path);
    printf("unlink(%s) -> %d\n", path, rv);
    return rv;
}

int
nufs_link(const char *from, const char *to)
{
    int rv = -1;

    int fromNum = tree_lookup(from);
    inode* parentNode = get_inode(directory_get_super(to));
    inode* fromNode = get_inode(fromNum);
    fromNode->refs += 1;

    rv = directory_put(parentNode, directory_get_name(to), fromNum);



    printf("link(%s => %s) -> %d\n", from, to, rv);
	return rv;
}

int
nufs_rmdir(const char *path)
{
    int rv = remove(path);
    printf("rmdir(%s) -> %d\n", path, rv);
    return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
    int rv = 0;
    //TODO: ERROR RV

    int from_node_num = tree_lookup(from);
    inode* from_node = get_inode(from_node_num);
    inode* from_parent_node = get_inode(directory_get_super(from));
    inode* to_parent_node = get_inode(directory_get_super(to));
    directory_put(to_parent_node, directory_get_name(to), from_node_num);
    directory_delete(from_parent_node, directory_get_name(from));




    printf("rename(%s => %s) -> %d\n", from, to, rv);
    return rv;
}

int
nufs_chmod(const char *path, mode_t mode)
{
    int rv = 0;

    if (tree_lookup(path) < 0) {
        rv = -1;
    }
    else {
        inode* node  = get_inode(tree_lookup(path));
	    node->mode = mode;
    }
    printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

int
nufs_truncate(const char *path, off_t size)
{
    int rv = 0;

    //TODO: use rv
    get_inode(tree_lookup(path))->size = size;

    printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
    return rv;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
    int rv = 0;
    printf("open(%s) -> %d\n", path, rv);
    return rv;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int rv = 6;

	inode* node  = get_inode(tree_lookup(path));

    int remainder = offset % PAGE_SIZE;
    int initialPage = offset / PAGE_SIZE;


    int currentPtr = 0;
    for (int i = initialPage; i < bytes_to_pages(size) + initialPage; i++) {
        char* data = pages_get_page(inode_get_pnum(node, i));

        //first pages' data read
        if (i == initialPage) {
            int length = PAGE_SIZE - remainder;
            memcpy(buf, data + remainder, length);
            currentPtr+=length;
        }
        else {
            memcpy(buf + currentPtr, data, PAGE_SIZE);
            currentPtr+=PAGE_SIZE;
        }
    }

    printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return currentPtr;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	
    inode* node  = get_inode(tree_lookup(path));

	grow_inode(node, size + offset);
	int currentPtr = 0;
	int initialPage = offset / PAGE_SIZE;
	int remainder = offset % PAGE_SIZE;
    
	for (int i = initialPage; i < bytes_to_pages(size) + initialPage; i++){
		
		char* data = pages_get_page(inode_get_pnum(node, i));
		
		if (i == initialPage) {
            int length = PAGE_SIZE - remainder;
			memcpy(data + remainder, buf + currentPtr, length);
			currentPtr += length;
		}
        else {
            memcpy(data, buf + currentPtr, PAGE_SIZE);
			currentPtr += PAGE_SIZE;
        }
	}	
	
	node->size = size + offset;
    node->time = time(0);
	int rv = size;
	
	printf("node size: %d\n", node->size);
    printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
}

// Update the timestamps on a file or directory.
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    int rv = 0;

    inode* node = get_inode(tree_lookup(path));
    node->time = ts[1].tv_sec;

    printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
           path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
	return rv;
}

// Extended operations
int
nufs_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi,
           unsigned int flags, void* data)
{
    int rv = -1;
    printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
    return rv;
}


int
nufs_symlink(const char* to, const char* from) {
    int rv = 0;
    rv = nufs_mknod(from, 0120000, 0);
    if (rv < 0) {
        return rv;
    }
    rv = nufs_write(from, to , strlen(to), 0, 0);
    printf("symlink(%s, %s) -> (%d)\n", from, to, rv);
    return rv;
}

int nufs_readlink(const char* path, char* buf, size_t size) {
    int rv = 0;
    rv = nufs_read(path, buf, size, 0 , 0);
    printf("readlink(%s) -> (%d)\n", path, rv);
    return rv;
}



void
nufs_init_ops(struct fuse_operations* ops)
{
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access   = nufs_access;
    ops->getattr  = nufs_getattr;
    ops->readdir  = nufs_readdir;
    ops->mknod    = nufs_mknod;
    ops->mkdir    = nufs_mkdir;
    ops->link     = nufs_link;
    ops->unlink   = nufs_unlink;
    ops->rmdir    = nufs_rmdir;
    ops->rename   = nufs_rename;
    ops->chmod    = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open	  = nufs_open;
    ops->read     = nufs_read;
    ops->write    = nufs_write;
    ops->utimens  = nufs_utimens;
    ops->ioctl    = nufs_ioctl;
    ops->readlink = nufs_readlink;
    ops->symlink  = nufs_symlink;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
    assert(argc > 2 && argc < 6);
    printf("TODO: mount %s as data file\n", argv[argc-1]);
    pages_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}

