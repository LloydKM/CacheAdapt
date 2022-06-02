#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#define STDIO 2

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include "cache_layer.h"

typedef int (*real_open_t)(const char *, int, ...);
typedef ssize_t (*real_read_t)(int, void *, size_t);
typedef int (*real_close_t)(int);

// TODO: parallel stuff

// TODO: find out which other sys calls need to be intercepted

int
real_open (const char *pathname, int flags, ...)
{
    return ((real_open_t)dlsym(RTLD_NEXT, "open"))(pathname, flags, NULL);
}

ssize_t
real_read (int fd, void *data, size_t size)
{
    // Do I need my own fd?
    return ((real_read_t)dlsym(RTLD_NEXT, "read"))(fd, data, size);
}

int
real_close (int fd)
{
    return ((real_close_t)dlsym(RTLD_NEXT, "close"))(fd);
}

/*
// maybe something is needed to bypass unwanted calls in gdb
long
ptrace(int request, int pid, void *addr, void *data)
{
    return 0L;
}
*/

int
open (const char *pathname, int flags, ...) 
{
    int err;
    int fd;
    char local_path[PATH_MAX];
    /* TODO: */

    if ((fd = real_open(pathname, flags, 0)) < 0)
    {
        // ERROR Handling and propagation
        return fd;
    }
    // Try not to "intercept" std input and output
    else if (fd <= STDIO)
    {
        // Just call real_open in case of STDIO
        return fd;
    }
    else
    {
        /* 
        - File probably needs handling, 
        - look up json with file information
        - check if already in cache
        - load necessery data into cache
        - create reference to data for faster laoding in read
        */
        printf("intercepted open: %s\n", pathname);
        if ((err = check_layer(pathname, local_path)) != 0)
        {
            // some kind of error handling later
            printf("file is not locally available\n");
            // Do I really wann load it "remotly" in this case? prob not
            // For the sake of simplicity it will be done this way for now
            // Prob doesn't slow down original program significantly
            return fd;
        }
        else
        {
            // file is locally available. Close original file. Open local one
            printf("file is locally available: %s\n", local_path);
            err = real_close(fd);
            fd = real_open(local_path, flags, 0);
            return fd;
        }
    }
}

ssize_t
read (int fd, void *data, size_t size)
{
    ssize_t amount_read;
    /* TODO: */

    // Try not to "intercept" std input and output
    if (fd > STDIO)
    {
        printf("intercepted read\n");
        /*
        - Some more Code TODO:
        - Read from own cache
            -> Maybe already done through my open implementation
                in most cases fd will already point to file in cache
        */
    }
    amount_read = real_read(fd, data, size);

    return amount_read;
}

int
close (int fd)
{
    int ret = 0;
    /* TODO: 
        - Do I need to delete file from cache if close is called?
        - Way to do this:
            call readlink on /proc/self/fd/NNN where NNN is the file descriptor
    */
    printf("intercepted close\n");
    ret = real_close(fd);

    return ret;
}