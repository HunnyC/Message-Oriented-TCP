CC = gcc
# CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -Wformat
AR = ar
ARFLAGS = rcs

all: libmsocket.a

libmsocket.a: msocket.o
	$(AR) $(ARFLAGS) $@ $^

msocket.o: msocket.c msocket.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.a
