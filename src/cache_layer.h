#ifndef CACHE_LAYER_H
#define CACHE_LAYER_H

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>

#include <glib.h>

#include "klib/khash.h"
#include "klib/kson.h"

#ifndef MAX_THREADS
    #define MAX_THREADS 4
#endif

#define KEY_PRESENT 0
#define KEY_MISSING 1

#define PARSE_SUCCESS   0
#define PARSE_FAIL      1

KHASH_MAP_INIT_STR(m32, const char *)
/* TODO:

Do I want other scripts to be able to access these? 
Option 1: Specific access through function calls
Option 2: private access only in this script
Option 3: Open access through header
*/
// Hard coded cache_path as default- -> Does this incur Option 3?
static const char *cache_path = "/tmp/CacheAdapt";
static khash_t(m32) *h;
static char buffer[PATH_MAX];

struct fd_info {
    int fdin;
    int fdout;
};

int _init_layer(const char *path);
char *_normalize_path(const char* path);
/*! @function
    @abstract           checks if file is already loaded
    @param path         path to file
    @param local_path   will store path to file if locally available
    @return             -1 error, 0 locally available, >0 not present
*/

void copy_to_tmp(gpointer data);
int check_layer(const char *path, char *local_path);

#endif