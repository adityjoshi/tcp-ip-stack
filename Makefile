# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Target executable
TARGET = my_program

# All source files
SRCS = graph.c testapp.c topologies.c  gluethread/glthread.c

# Default rule to compile everything into one executable
all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Clean rule to remove the executable
clean:
	rm -f $(TARGET)
