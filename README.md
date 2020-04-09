# NUFS Filesystem

_Created in CS3650, Computer Systems with a partner._

Designed a working file system that uses inodes to keep track of disk space.

Functionality:

    - Read and write from files larger than one block. 
    - Create directories and nested directories. Directory depth should only be limited by disk space (and possibly the POSIX API).
    - Remove directories.
    - Hard links.
    - Symlinks
    - Support modification and display of metadata (permissions and timestamps) for files and directories.
    - Create files.
    - List the files in the filesystem root directory (where you mounted it).
    - Delete files.
    - Rename files.

Unsupported: Multiple users.

File system is built on the FUSE interface on Debian.
The system is designed to be a tree of directories, each containing
their own files.
Pages are created by memory mapping and return the allocated space.
Each directory has a corresponding inode in memory that holds pointers to
it and other metadata such as timestamps and permissions.