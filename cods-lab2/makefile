CC=gcc
CFLAGS=-Wall -lpthread -pthread

SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))

.PHONY: all
all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean: 
	rm -f $(PROGS)

