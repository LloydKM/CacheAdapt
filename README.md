# Warning NOT current

## Compile dynamic library
```bash
cd src/
gcc -shared -fPIC -o iohooks.so iohooks.c
# compile for debugging
gcc -shared -g -Og -fPIC -o iohooks.so iohooks.c
```

## Compile test program
```bash
cd src/
gcc main.c -o out
# compile for debugging
gcc main.c -g -Og -o out
```

## run program
```bash
LD_PRELOAD=$PWD/iohooks.so ./out ../random
```

## Use gdb with iohooks
```bash
gdb your_program

(gdb) set environment LD_PRELOAD ./iohooks.so
(gdb) start
```