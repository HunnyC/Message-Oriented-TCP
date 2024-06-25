CC = gcc
# CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -Wformat
LDFLAGS = -pthread

all: initmsocket

initmsocket: initmsocket.o libmsocket.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

initmsocket.o: initmsocket.c msocket.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o initmsocket
