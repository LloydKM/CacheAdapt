CC		= gcc
CFLAGS	= -std=c11 -Wall -Wextra -Wpedantic -O0
DEBUG	= -std=c11 -Wall -Wextra -Wpedantic -Og -g
SRC 	= $(CURDIR)/src
LIBS	= -I$(CURDIR)/lib

# flags to link needed libraries
LIBS += -ldl

# cflags and libs for glib-2.0
CFLAGS  += -I/opt/spack/20221019/spack/opt/spack/linux-centos8-x86_64_v3/gcc-12.2.0/glib-2.74.0-gzw7imf226ultlmv26a2sj54kd3ej3ex/include/glib-2.0 -I/opt/spack/20221019/spack/opt/spack/linux-centos8-x86_64_v3/gcc-12.2.0/glib-2.74.0-gzw7imf226ultlmv26a2sj54kd3ej3ex/lib/glib-2.0/include
LIBS	+= -Wl,-rpath=/opt/spack/20221019/spack/opt/spack/linux-centos8-x86_64_v3/gcc-12.2.0/gettext-0.21-qtxqirwir3t2yl2x2mk6znobsgbxaeuj/lib -lglib-2.0 -lintl 

iohooks.so: $(SRC)/iohooks.c $(SRC)/iohooks.h cache_layer.o kson.o
	$(CC) $(CFLAGS) -shared $(LIBS) -fPIC -o iohooks.so \
	$(SRC)/iohooks.c cache_layer.o kson.o

cache_layer.o: $(SRC)/cache_layer.c $(SRC)/cache_layer.h
	$(CC) $(CFLAGS) $(LIBS) -fPIC -c $(SRC)/cache_layer.c

kson.o: $(CURDIR)/lib/klib/kson.c $(CURDIR)/lib/klib/kson.h
	$(CC) $(CFLAGS) $(LIBS) -fPIC -c $(CURDIR)/lib/klib/kson.c

test: $(SRC)/main.c
	$(CC) $(CFLAGS) -o test $(SRC)/main.c

all: iohooks.so test

clean:
	$(RM) test cache_layer.o iohooks.so $(SRC)/out $(SRC)/iohooks.so \
	kson.o
