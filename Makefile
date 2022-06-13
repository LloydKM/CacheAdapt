CC		= gcc
CFLAGS	= -std=c11 -Wall -Wextra -Wpedantic -O0
DEBUG	= -std=c11 -Wall -Wextra -Wpedantic -Og -g
SRC 	= $(CURDIR)/src
LIBS	= -I$(CURDIR)/lib

iohooks.so: $(SRC)/iohooks.c $(SRC)/iohooks.h cache_layer.o
	$(CC) $(CFLAGS) -shared $(LIBS) -fPIC -o iohooks.so \
	$(SRC)/iohooks.c cache_layer.o

cache_layer.o: $(SRC)/cache_layer.c $(SRC)/cache_layer.h
	$(CC) $(CFLAGS) $(LIBS) -c $(SRC)/cache_layer.c

test: $(SRC)/main.c

clean:
	$(RM) test cache_layer.o iohooks.so $(SRC)/out $(SRC)/iohooks.so