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