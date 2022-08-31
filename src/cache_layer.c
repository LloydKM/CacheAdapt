/* 
library that implements and manages the caching layer for the intercepted
system calls. 
*/
#include "cache_layer.h"
#include "iohooks.h"

static GThreadPool *thread_pool;
static GError *error;
//static int files[1000];
int *files;
static bool is_initialized = false;

// declare some internal functions
int ca_init_layer(const char *path);
char *ca_normalize_path(const char* path);

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
ca_init_layer(const char *path)
{
    // TODO: Create queue for loading of files
    g_message("initializing CacheAdapt caching layer");
    khint_t iterator;
    gboolean exclusive = TRUE;
    //allocate array for filepointer
    files = (int*)malloc(1000*sizeof(int));
    // allocate a hash table
    h = kh_init(m32);
    // initialize thread pool used by layer
    thread_pool = g_thread_pool_new((GFunc) ca_copy_to_tmp,
                                    NULL,
                                    MAX_THREADS,
                                    exclusive,
                                    &error);
    is_initialized = true;
    g_message("CacheAdapt caching layer initialized");
    return 0;
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

// This needs to be called concurrently
// probably omp in extra thread
int
ca_load_adjacent_files(const char *path)
{
    kson_t *kson = 0;
    gint g_err;
    mode_t mode = 0777;
    int ret, i;
    FILE *fp;

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
        char local_path[PATH_MAX];
        int fdin, fdout;
        int err;
        gboolean pushed;
        khint_t iterator;

        //files = malloc(sizeof(struct fd_info));

        p = kson_by_key(kson->root, "adjacent_files");
        for (i = 0; (adj_file = kson_by_index(p, i)); i++)
        {
            resolved_path = realpath(adj_file->v.str, NULL);
            // open file
            if ((fdin = real_open(resolved_path, O_RDWR, mode)) != 0)
            {
                //files->fdin = fdin;
                files[2*i] = fdin;
                strncpy(local_path, ca_normalize_path(resolved_path), PATH_MAX);
                // crete path in tmp (glib)
                g_err = g_mkdir_with_parents(local_path, mode);
                rmdir(local_path);
                // create and open file in tmp
                fdout = real_open(local_path, O_RDWR | O_CREAT | O_TRUNC, mode);
                files[2*i + 1] = fdout;
                // put entry into hash_table
                iterator = kh_put(m32, h, resolved_path, &ret);
                kh_value(h, iterator) = local_path;
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
                free(resolved_path);
                
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
    g_message("fdin: %d and fdout: %d", fdin, fdout);

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
    /* TODO:

    - return hash key?
    */
    g_info("entered cache_layer:ca_check_layer");
    if (!is_initialized)
    {
        ret = ca_init_layer(path);

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
        iterator = kh_put(m32, h, path, &ret);
        g_info("cache_layer:ca_check_layer: inserting key succeeded");
        strncpy(local_path, ca_normalize_path(path), PATH_MAX);
        kh_value(h, iterator) = local_path;
        g_info("cache_layer:ca_check_layer: inserting value (%s) succeeded", local_path);
        // Needs to be called concurently
        if ((err = ca_load_adjacent_files(path)) != PARSE_SUCCESS)
        {
            g_warning("Couldn't laod adjacent files");
        }
    }

    return ret;
}
