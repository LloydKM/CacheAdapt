#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#define STDIO 2

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

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
    return ((real_read_t)dlsym(RTLD_NEXT, "read"))(fd, data, size);
}

int
real_close (int fd)
{
    return ((real_close_t)dlsym(RTLD_NEXT, "close"))(fd);
}


int
open (const char *pathname, int flags, ...) 
{
    int ret = -1;
    /* TODO: */

    // Try not to "intercept" std input and output
    if ((ret = real_open(pathname, flags, 0)) < 0)
    {
        // ERROR Handling and propagation
        return ret;
    }
    else if (ret <= STDIO)
    {
        // Just call real_open in case of STDIO
        return ret;
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
        return ret;
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
        */
    }
    amount_read = real_read(fd, data, size);

    return amount_read;
}

int
close (int fd)
{
    int ret = 0;
    /* TODO: */
    printf("intercepted close\n");
    ret = real_close(fd);

    return ret;
}