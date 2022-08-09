CC		= gcc
CFLAGS	= -std=c11 -Wall -Wextra -Wpedantic -O0
DEBUG	= -std=c11 -Wall -Wextra -Wpedantic -Og -g
SRC 	= $(CURDIR)/src
LIBS	= -I$(CURDIR)/lib `pkg-config --cflags --libs glib-2.0`

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