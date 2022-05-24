#ifndef CACHE_LAYER_H
#define CACHE_LAYER_H

#include <stdbool.h>
#include <stdio.h>

// Don't forget include path for compilation -Ilib
#include "klib/khash.h"

KHASH_MAP_INIT_STR(m32, const char *)
/* TODO:

Do I want other scripts to be able to access these? 
Option 1: Specific access through function calls
Option 2: private access only in this script
Option 3: Open access through header
*/
// Hard coded cache_path as default- -> Does this incur Option 3?
static const char *cache_path = "/tmp/CacheAdapt";
static bool is_initialized = false;
static khash_t(m32) *h;

int init_layer(const char *path);
int check_layer(const char *path);

#endif