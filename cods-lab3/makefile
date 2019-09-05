CC=gcc
CFLAGS=-Wall -lpthread -pthread
DEBUG=-g

SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))

.PHONY: all
all: $(PROGS)

%: %.c
	$(CC) $(DEBUG) $(CFLAGS) -o $@ $<

.PHONY: clean
clean: 
	rm -f $(PROGS)

