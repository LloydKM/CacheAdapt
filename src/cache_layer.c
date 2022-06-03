/* 
library that implements and manages the caching layer for the intercepted
system calls. 
*/
#include "cache_layer.h"

static bool is_initialized = false;

/* TODO:

What functions do I need?
+ something to initialize layer
- something to store/check file
- something to load adjacent files
- something to access file ????
- something to create link or hash name (like a file handle)
- something to free allocated space
*/

int
_init_layer(const char *path)
{
    printf("initialize cache layer\n");
    khint_t iterator;
    // allocate a hash table
    h = kh_init(m32);
    is_initialized = true;
    return 0;
}

char *
_normalize_path(const char* path)
{
    char resolved_path[PATH_MAX];
    printf("normalizing path\n");
    realpath(path, resolved_path);
    char *local_path = strcat(strcpy(buffer, cache_path), resolved_path);
    printf("normalized to: %s\n", local_path);
    return local_path;
}

int
check_layer(const char *path, char *local_path)
{
    int ret;
    bool is_missing;
    khint_t iterator;
    /* TODO:

    - return hash key?
    */
    printf("entered cache_layer:check_layer\n");
    if (!is_initialized)
    {
        ret = _init_layer(path);

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
        /* TODO: if entry exists

        - get path to local file
        */
        // Maybe needs error handling
        ret = KEY_PRESENT;
        printf("cache_layer:check_layer: trying to load existing entry\n");
        strncpy(local_path, kh_value(h, iterator), PATH_MAX);
        printf("cache_layer:check_layer: load of existing file succeeded\n");
    }
    else
    {
        /* TODO: if entry is missing

        + store link in table
        - load file and adjacent file to local drive
        - create entries for the adjacent files in table
        */
        ret = KEY_MISSING;
        printf("cache_layer:check_layer: inserting path into hash table\n");
        iterator = kh_put(m32, h, path, &ret);
        printf("cache_layer:check_layer: inserting key succeeded\n");
        strncpy(local_path, _normalize_path(path), PATH_MAX);
        kh_value(h, iterator) = local_path;
        printf("cache_layer:check_layer: inserting value (%s) succeeded\n", local_path);
    }

    return ret;
}