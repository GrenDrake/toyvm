OBJS=toyvm.o vmcore.o
TARGET=./toyvm

AOBJS=assemble.o assem_tokens.o assem_build.o assem_labels.o utility.o
ATARGET=./assemble

CC=gcc
CFLAGS=-Wall -std=c99 -pedantic

all: $(TARGET) $(ATARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

$(ATARGET): $(AOBJS)
	$(CC) $(AOBJS) -o $(ATARGET)

clean:
	rm *.o $(TARGET) $(ATARGET)

.PHONY: all clean
