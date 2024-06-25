CC = gcc
# CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -O2
LDFLAGS = -L. -lmsocket
TARGETS = user1 user2

all: $(TARGETS)

user1: user1.c libmsocket.a
	@echo "[CC] $<"
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

user2: user2.c libmsocket.a
	@echo "[CC] $<"
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	@echo "[RM] $(TARGETS)"
	rm -f $(TARGETS)

