/* 
library that implements and manages the caching layer for the intercepted
system calls. 
*/
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

/* TODO

Do I want other scripts to be able to access these? 
Option 1: Specific access through function calls
Option 2: private access only in this script
Option 3: Open access through header
*/
// Hard coded cache_path as default- -> Does this incur Option 3?
static const char *cache_path = "/tmp/CacheAdapt";

/* TODO

What functions do I need?
- something to store file
- something to access file
- something to create link or hash name (like a file handle)
*/
int
store()