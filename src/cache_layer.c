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

// This needs to be called concurrently
// omp or extra thread
int
_load_adjacent_files(const char *path)
{
    kson_t *kson = 0;
    int ret, tmp, i;
    int len = 0;
    int max = 0;
    FILE *fp;
    char *json = 0;
    char buf[0x10000];

    // TODO: Search for json based on path

    if ((fp = fopen("./config/test.json", "rb")) != 0)
    {
        // read file into string
        while ((tmp = fread(buf, 1, 0x10000, fp)) != 0)
        {
            if (len + tmp + 1 > max)
            {
                max = len + tmp + 1;
                kroundup32(max);
                json = (char *) realloc(json, max);
            }
            memcpy(json + len, buf, tmp);
            len += tmp;
        }
        fclose(fp);
        // parse
        kson = kson_parse(json);
        free(json);
        if (kson)
        {
            const kson_node_t *p, *adj_file;
            // kson_format(kson->root);
            p = kson_by_key(kson->root, "adjacent_files");
            for (i = 0; (adj_file = kson_by_index(p, i)); i++)
            {
                //adj_file = kson_by_index(p, i);
                // open file
                // crete path in tmp (glib)
                // create and open file in tmp
                // call copy_to_tmp
                printf("%s should be laoded here\n", adj_file->v.str);
            }
            ret = PARSE_SUCCESS;
        } 
        else 
        {
            ret = PARSE_FAIL;
            printf("failed to parse\n");
        }
    }

    return ret;
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


void
copy_to_tmp(const char *pathname, const char *local_path, int fdin, int fdout)
{
    void *src, *dst;
    struct stat statbuf;

    printf("copy_to_tmp called\n");

    if (fstat(fdin, &statbuf) < 0)
    {
        printf("fstat error");
        return;
    }
    printf("mmap original\n");
    if ((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0))
        == (caddr_t) -1)
    {
        printf("mmap error for input");
        return;
    }

    printf("mmap local\n");
    if ((dst = mmap (0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
        fdout, 0)) == (caddr_t) -1)
    {
        printf("mmap error for output\n");
        printf("local_path: %s\n", local_path);
        printf("errno: %d\n", errno);
        //return;
    }
    printf("copy contents to local file\n");
    //memcpy(dst, src, statbuf.st_size);
    ssize_t nwritten = write(fdout, src, statbuf.st_size);
    printf("end of copy_to_file reached\n");
}

int
check_layer(const char *path, char *local_path)
{
    int ret, err;
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
        // Needs to be called concurently
        if ((err = _load_adjacent_files(path)) != PARSE_SUCCESS)
        {
            printf("Couldn't laod adjacent files\n");
        }
    }

    return ret;
}