
CC = gcc

SRCS += ringfifo.c
SRCS += test.c


CFLAGS += -O2

LDFLAGS += -lpthread


TARGET += ringfifo_test

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

