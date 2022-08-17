# Prerequisites
## IMPORTANT before using make
To use this makefile you have to change following lines in the Makefile:
```bash
# cflags and libs for glib-2.0
CFLAGS  += -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/sysprof-4 -pthread
LIBS	+= -lglib-2.0
```
Change the arguments on the righthand side to the results of
```bash
pkg-config --cflags --libs glib-2.0
pkg-config --libs glib-2.0
```

# Compile library
## Compile dynamic library
```bash
make iohooks.so
# compile for debugging
cd src/
gcc -shared -g -Og -I../lib -fPIC -o iohooks.so iohooks.c cache_layer.c
```

## Compile test program
```bash
make test
# compile for debugging
cd src/
gcc main.c -g -Og -o out
```

# Using Library
## run program
```bash
LD_PRELOAD=$PWD/iohooks.so your_program
```

## Use gdb with iohooks
```bash
gdb --args your_program

(gdb) set environment LD_PRELOAD ./iohooks.so
(gdb) start
```