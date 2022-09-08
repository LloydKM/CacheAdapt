#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#define STDIO 2

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <stdio.h>
//#include <sys/mman.h>
//#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#define G_LOG_USE_STRUCTURED

#include <glib.h>

#include "cache_layer.h"

typedef int (*real_open_t)(const char *, int, ...);
typedef ssize_t (*real_read_t)(int, void *, size_t);
typedef int (*real_close_t)(int);

// TODO: find out which other sys calls need to be intercepted

int
real_open (const char *pathname, int flags, ...)
{
    if ((flags & O_CREAT) == O_CREAT)
    {
        /*
        char dir_name[PATH_MAX];
        strncpy(dir_name, pathname, PATH_MAX);
        printf("length of %s: %d\n", dir_name, strlen(dir_name));
        for (size_t i = strlen(dir_name) - 1; dir_name[i] != '/'; i--)
        {
            printf("real_open: delete %c\n", dir_name[i]);
            dir_name[i] = '\0';
        }
        strcat(dir_name, "XXXXXX");
        printf("real_open: dir_name is %s\n", dir_name);
        mkdtemp(dir_name);
        */

        return ((real_open_t)dlsym(RTLD_NEXT, "open"))(pathname, flags, 0x0777);    
    }
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
    gint g_err;
    int fdin, fdout;
    mode_t mode = 0777;
    char local_path[PATH_MAX];
    struct fd_info *files;
    
    /* TODO:
        - Does this check significantly impact performance?
    */
    if (!(FNM_NOMATCH == fnmatch("*/config",
                                 pathname,
                                 FNM_LEADING_DIR)))
    {
        g_info("Intercepted config file - No need for handling");
        return real_open(pathname, flags, 0);
    }

    if ((fdin = real_open(pathname, flags, 0)) < 0)
    {
        // ERROR Handling and propagation
        return fdin;
    }
    // Try not to "intercept" std input and output
    else if (fdin <= STDIO)
    {
        // Just call real_open in case of STDIO
        g_info("STDIO called for read");
        return fdin;
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
        // TODO: never freed
        files = malloc(sizeof(struct fd_info));
        files->fdin = fdin;

        g_info("intercepted open: %s", pathname);
        if ((err = ca_check_layer(pathname, local_path)) != 0)
        {
            // some kind of error handling later
            g_info("file is not locally available");
            // Do I really wann load it "remotly" in this case? prob not
            // For the sake of simplicity it will be done this way for now
            // Prob doesn't slow down original program significantly
            g_err = g_mkdir_with_parents(local_path, mode);
            err = rmdir(local_path);
            fdout = real_open(local_path, O_RDWR | O_CREAT | O_TRUNC, mode);
            if (fdout == -1) {
                g_error("Couldn't create local file errno: %d", errno);
            }
            files->fdout = fdout;
            printf("opened %s with fd: %d\n", local_path, fdout);
            // TODO: Changed copy to temp to work with array(Not this struct). Current state unsafe
            ca_copy_to_tmp((gpointer) files);
            // TODO: Are these shenanigans necessary?
            err = real_close(fdin);
            return fdout;
        }
        else
        {
            // file is locally available. Close original file. Open local one
            g_info("file is locally available: %s", local_path);
            fdout = real_open(local_path, O_RDWR | O_CREAT | O_TRUNC, mode);
            files->fdout = fdout;
            //ca_copy_to_tmp((gpointer) files);
            err = real_close(fdin);
            return fdout;
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
        g_info("intercepted read");
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
        - Maybe use poolthread with low priority so IO doesn't halt program
    */
    g_info("intercepted close");
    ret = real_close(fd);

    return ret;
}
