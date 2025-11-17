# This Makefile build bench with source files

# The compiler: gcc for C programs
CC = gcc

# Compiler flags:
# -Wall for debugger warnings
# -std=c11 for updated standards
CFLAGS = -Wall -std=c11

# The name of the program that we are producing.
TARGET = bench

# The target to build
all: $(TARGET)

# The executable
$(TARGET): bench.o mem_utils.o memory.o
	$(CC) $(CFLAGS) -g -o bench bench.o mem_utils.o memory.o

# Individual source files
bench.o: bench.c mem.h mem_impl.h
	$(CC) $(CFLAGS) -g -c bench.c

mem_utils.o: mem_utils.c mem.h mem_impl.h
	$(CC) $(CFLAGS) -g -c mem_utils.c

memory.o: memory.c mem.h mem_impl.h
	$(CC) $(CFLAGS) -g -c memory.c

test: bench
	./bench

# A "phony" target to remove built files and backups
clean:
	rm -f *.o bench *~
