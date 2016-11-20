
CC = gcc

SRCS += ringfifo.c
SRCS += test.c


CFLAGS += 

LDFLAGS += -lpthread


TARGET += ringfifo_test

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

