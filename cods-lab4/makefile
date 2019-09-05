CC=gcc
CFLAGS=-Wall -lpthread -pthread
LFLAGS=-lm
DEBUG=-g

SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))

.PHONY: all
all: $(PROGS)

%: %.c
	$(CC) $(DEBUG) $(CFLAGS) -o $@ $< $(LFLAGS)

.PHONY: clean
clean: 
	rm -f $(PROGS)

