/* 
library that implements and manages the caching layer for the intercepted
system calls. 
*/
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "cache_layer.h"

/* TODO:

What functions do I need?
- something to initialize layer
- something to store/check file
- something to load adjacent files
- something to access file ????
- something to create link or hash name (like a file handle)
*/

int
init_layer(const char *path)
{
    khint_t iterator;
    // allocate a hash table
    h = kh_init(m32);
    return 0;
}

int
check_layer(const char *path)
{
    int ret;
    bool is_missing;
    khint_t iterator;
    /* TODO:

    - return hash key?
    */
    if (!is_initialized)
    {
        ret = init_layer(path);

        if (ret < 0)
        {
            perror("Couldn't initialize layer");
            return -1;
        }
    }

    // check for match in table
    iterator = kh_get(m32, h, path);
    is_missing = (iterator == kh_end(h));

    if(!is_missing) 
    {
        // TODO: if entry exists
    }
    else
    {
        // TODO: if entry is missing
    }

    return 0;
}