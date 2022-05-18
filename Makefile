CC		= gcc
CFLAGS	= -std=c11 -Wall -Wextra -Wpedantic -O0 -g
# LDLIBS =

ld_preload: 
test: $(CURDIR)/main.c $(CURDIR)/iohooks.c

clean:
	$(RM) test