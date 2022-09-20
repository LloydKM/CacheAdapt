/* 
library that implements and manages the caching layer for the intercepted
system calls. 
*/
#include "cache_layer.h"
#include "iohooks.h"

static GThreadPool *thread_pool;
static GError *error = NULL;
int *files;
static bool is_initialized = false;

// declare some internal functions
void ca_init_layer(const char *path);
char *ca_normalize_path(const char* path);
void clean_up(void);

/* TODO:

What functions do I need?
+ something to initialize layer
- something to store/check file
- something to load adjacent files
- something to access file ????
- something to create link or hash name (like a file handle)
- something to free allocated space
*/

void
ca_init_layer(const char *path)
{
    (void) path;

    g_message("initializing CacheAdapt caching layer");
    gboolean exclusive = TRUE;
    // allocate a hash table
    h = kh_init(m32);
    //allocate array for filepointer used in threadpool
    files = (int*)malloc(1000*sizeof(int));
    // initialize thread pool used by layer
    thread_pool = g_thread_pool_new((GFunc) ca_copy_to_tmp,
                                    NULL,
                                    MAX_THREADS,
                                    exclusive,
                                    &error);
    atexit(&clean_up);
    if (error)
    {
        g_message("Failed to initialize Thread Pool: %s", error);
        g_error_free(error);
        g_error("Error initializing Thread Pool: %s");
    }
    else
    {
        is_initialized = true;
        g_message("CacheAdapt caching layer initialized");
    }
}

void
ca_parse_json(FILE *src, kson_t **dest)
{
    size_t tmp;
    size_t len = 0;
    size_t max = 0;
    char *json= 0;
    char buf[0x10000];

    // read file into string
    while ((tmp = fread(buf, 1, 0x10000, src)) != 0)
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

    *dest = kson_parse(json);
    free(json);
}

int
ca_load_adjacent_files(const char *path)
{
    kson_t *kson = 0;
    gint g_err;
    mode_t mode = 0777;
    int ret, i;
    FILE *fp;

    (void) path;
    // TODO: Search for json based on path
    // Maybe seperate json handling from rest of function // modularity

    if ((fp = fopen("./config/test.json", "rb")) != 0)
    {
        ca_parse_json(fp, &kson);
        fclose(fp);
    }
    if (kson)
    {
        const kson_node_t *p, *adj_file;
        char *resolved_path;
        char *local_path;
        int fdin, fdout;
        size_t len;
        int err;
        gboolean pushed;
        khint_t iterator;

        // TODO: Loop shouldn't be entered as long as there are still jobs in thread pool
        p = kson_by_key(kson->root, "adjacent_files");
        for (i = 0; (adj_file = kson_by_index(p, i)); i++)
        {
            resolved_path = realpath(adj_file->v.str, NULL);
            // g_message("resolved_path: %s", resolved_path);
            // open file
            if ((fdin = real_open(resolved_path, O_RDWR, mode)) != 0)
            {
                //files->fdin = fdin;
                files[2*i] = fdin;
                //strncpy(local_path, ca_normalize_path(resolved_path), PATH_MAX);
                strncpy(buffer, ca_normalize_path(resolved_path), PATH_MAX);
                len = strlen(buffer);
                local_path = (char *)malloc(sizeof(char)*(len+1));
                strncpy(local_path, buffer, len+1);
                g_info("local_path: %s", local_path);
                // crete path in tmp (glib)
                g_err = g_mkdir_with_parents(local_path, mode);
                rmdir(local_path);
                // create and open file in tmp
                fdout = real_open(local_path, O_RDWR | O_CREAT | O_TRUNC, mode);
                files[2*i + 1] = fdout;
                // put entry into hash_table
                iterator = kh_put(m32, h, strdup(resolved_path), &ret);
                kh_value(h, iterator) = strdup(local_path);
                // call ca_copy_to_tmp
                if ((pushed = g_thread_pool_push(thread_pool,
                                            (gpointer) &(files[i*2]),
                                            g_err))
                     != TRUE)
                {
                    g_error("couldn't push new task to thread pool");
                }
                //ca_copy_to_tmp(fdin, fdout);
                //err = real_close(fdin);
                //err = real_close(fdout);
                //free(resolved_path);
                
            }
            g_info("%s should be laoded here", adj_file->v.str);
        }
        //free(files);
        ret = PARSE_SUCCESS;
    } 
    else 
    {
        ret = PARSE_FAIL;
        g_warning("failed to parse");
    }

    return ret;
}

char *
ca_normalize_path(const char* path)
{
    char resolved_path[PATH_MAX];
    g_info("normalizing path");
    realpath(path, resolved_path);
    char *local_path = strcat(strcpy(buffer, cache_path), resolved_path);
    g_info("normalized to: %s", local_path);
    return local_path;
}


void
ca_copy_to_tmp(gpointer data)
{
    void *src, *dst;
    struct stat statbuf;

    //struct fd_info *files = ((struct fd_info *) data);

    int fdin = *((int *) data);
    int fdout = *(((int *) data) + 1);
    //g_message("fdin: %d and fdout: %d", files->fdin, files->fdout);
    g_info("fdin: %d and fdout: %d", fdin, fdout);

    g_info("ca_copy_to_tmp called");

    if (fstat(fdin, &statbuf) < 0)
    {
        g_error("fstat error: %d", errno);
    }
    g_info("mmap original");
    if ((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0))
        == (caddr_t) -1)
    {
        g_error("mmap error for input: %d", errno);
    }

    g_info("mmap local");
    if ((dst = mmap (0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
        fdout, 0)) == (caddr_t) -1)
    {
        g_error("mmap error for output\n"
                "errno: %d", errno);
    }
    g_info("copy contents to local file");
    //memcpy(dst, src, statbuf.st_size);
    ssize_t nwritten = write(fdout, src, statbuf.st_size);
    g_info("end of copy_to_file reached");
}

int
ca_check_layer(const char *path, char *local_path)
{
    int ret, err;
    bool is_missing;
    khint_t iterator;
    char* resolved_path;
    /* TODO:

    - return hash key?
    */
    g_info("entered cache_layer:ca_check_layer");
    if (!is_initialized)
    {
        ca_init_layer(path);
    }

    // check for match in table
    // TODO: cleanup usage of realpath()
    resolved_path = realpath(path, NULL);
    iterator = kh_get(m32, h, resolved_path);
    is_missing = (iterator == kh_end(h));
    g_info("I exist, is_missing = %d, %s", is_missing, path);

    if(!is_missing) 
    {
        /* TODO: if entry exists

        - get path to local file
        */
        // Maybe needs error handling
        ret = KEY_PRESENT;
        g_info("cache_layer:ca_check_layer: trying to load existing entry");
        strncpy(local_path, kh_value(h, iterator), PATH_MAX);
        g_info("cache_layer:ca_check_layer: load of existing file succeeded");
    }
    else
    {
        /* TODO: if entry is missing

        + store link in table
        - load file and adjacent file to local drive
        - create entries for the adjacent files in table
        */
        ret = KEY_MISSING;
        g_info("cache_layer:ca_check_layer: inserting path into hash table");
        iterator = kh_put(m32, h, strdup(path), &ret);
        g_info("cache_layer:ca_check_layer: inserting key succeeded");
        strncpy(local_path, ca_normalize_path(path), PATH_MAX);
        kh_value(h, iterator) = strdup(local_path);
        g_info("cache_layer:ca_check_layer: inserting value (%s) succeeded", local_path);
        // Needs to be called concurently
        if ((err = ca_load_adjacent_files(path)) != PARSE_SUCCESS)
        {
            g_warning("Couldn't laod adjacent files");
        }
    }

    return ret;
}

void
print_hashtable(void)
{
    khiter_t k;
    for (k = kh_begin(h); k != kh_end(h); ++k)
    {
        if (kh_exist(h, k))
        {
            printf("%s - %s\n", kh_key(h, k), kh_value(h, k));
        }
    }
}

void
clean_up(void)
{
    g_message("cleaning up");
    g_thread_pool_free(thread_pool, TRUE, TRUE);
    free(files);
    g_message("hash-table size: %d", kh_size(h));
    g_message("hash-table number of buckets: %d", kh_n_buckets(h));
    print_hashtable();
    kh_destroy(m32, h);
}
