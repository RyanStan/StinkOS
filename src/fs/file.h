/* file.h
 *
 * Our VFS interface
 * The actual concrete filesystems must implement these functions
 * 
 * We have significantly fewer levels of abstraction in our vfs than the Linux kernel 
 * (i.e. no dentry, inode, etc...)
 */

#ifndef FILE_H
#define FILE_H

#include "fs/pparser.h"
#include <stdint.h>

#define FS_NAME_MAX     20

enum file_seek_mode {
        SEEK_SET,
        SEEK_CUR,
        SEEK_END
};

enum file_mode {
        READ,           //O_RDONLY
        WRITE,          //O_WRONLY
        APPEND,         //O_APPEND
        INVALID         //??
};

/* We need to forward declare disk since disk.h includes this file */
struct disk;            

/* Each concrete file system driver implementation in our OS will have an associated filesystem struct instance */
struct filesystem {

        char name[FS_NAME_MAX];

        /* fs_open - stream open function 
         *
         * Opens the file whose path is the value contained in list beginning with path_part and associates
         * a stream with it.  This will return filesystem implementation specific private data.
         */
        void *(*fs_open)(struct disk *disk, struct path_part *path_part, enum file_mode mode);

        /* resolve
         *
         * returns true if the disk is formatted
         * to the specification of this function's implementation filesystem's format
         * 
         * E.g. if the function/filesystem implementing this function pointer is part of our
         * FAT filesystem driver, then it will return true if disk is formatted as a FAT filesystem
         */
        int (*resolve)(struct disk *disk); 

        /* fs_read - binary stream input
         *
         * Reads nmemb items of data, each size bytes long, 
         * from the stream associated with private (private is often a file descriptor),    --> TODO: double check that private is probably a file descriptor structure e.g. fat_file_descriptor
         * storing them at the location given by out
         */
        int (*fs_read)(struct disk *disk, void *private, uint32_t size, uint32_t nmemb, char *out);
};

/* File descriptor that represents an open file */
struct file_descriptor {

        /* Corresponds to the index of the file descriptor within the array of all file descriptors (file_descriptors) */
        int index;
        
        /* In the Linux kernel, a file descriptor is tied to a dentry object which is tied to an inode 
         * which is tied to a super block which is tied to a filesystem_type.  For our kernel,
         * we're linking the file directly to the filesystem_type (or just filesystem in our case).
         */
        struct filesystem *filesystem;

        struct disk *disk;

        /* private will point to driver implementation specific data
         * which will be used by other functions for finding the file on disk
         * This field will be set by fs_open
         */
        void *private;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Our VFS layer functions */

/* Initializes all file systems.
 * Loads compile time filesystem drivers.
 */
void fs_init();

/* Dynamically inserts a filesystem.  This loads our OS with the file system driver so that
 * future filesystems of the corresponding type can be mounted to our OS.  
 */
void fs_insert_filesystem(struct filesystem *filesystem);

/* Responsible for locating the correct filesystem to open a file with.
 * Calls the fopen function corresponding with the filesystem that owns the file.
 * 
 * mode_str - string that specifies the mode the file should be opened in
 *               'r' = read, 'w' = write, 'a' = append
 * filename - absolute path to the file
 * 
 * Returns the file descriptor index associated with filename (non-negative integer) on success
 * Returns < 0 on failure
 */
int fopen(const char *filename, const char *mode_str);

/* Finds a filesystem that can manage the disk.  Calls that respective filesystem's resolve
 * function on the disk.  
 * Returns a pointer to the filesystem on success or 0 on failure.
 * TODO: the way we're using this right now is odd.  We're calling it within disk_search_and_init to assign the disk's filesystem.
 *       However, it seems like since the filesystem's resolve method is passed the disk, it should just set the disk's filesystem itself
 */
struct filesystem *fs_resolve(struct disk *disk);

/* fread - binary stream input - VFS layer
 * 
 * Reads nmemb items of data, each size bytes long, 
 * from the stream associated with private (private is often a file descriptor),
 * storing them at the location given by out
 * 
 * This function calls the fread function of the filesystem implementation that is associated with disk.
 * 
 * Returns 0 on success or < 0 on failure
 */
//int fread(struct disk *disk, void *private, uint32_t size, uint32_t nmemb, char *out);
// TODO: comment with why fread uses following parameters and what they mean. ptr will be location of output buffer
int fread(void *ptr, uint32_t size, uint32_t nmemb, int fd);

#endif