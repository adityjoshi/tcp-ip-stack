# # Compiler and flags
# CC = gcc
# CFLAGS = -Wall -Wextra -g
# LIBS=-lpthread -L ./CommandParser -lcli

# # Target executable
# TARGET = my_program

# # All source files
# SRCS = graph.c testapp.c topologies.c  gluethread/glthread.c net.c

# # Default rule to compile everything into one executable
# all:
# 	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# # Clean rule to remove the executable
# clean:
# 	rm -f $(TARGET)

CC = gcc
CFLAGS = -g
TARGET = test.exe
LIBS = -lpthread -L ./CommandParser -lcli
OBJS = gluethread/glthread.o \
       graph.o \
       topologies.o \
       net.o

$(TARGET): testapp.o $(OBJS) CommandParser/libcli.a
	$(CC) $(CFLAGS) testapp.o $(OBJS) -o $(TARGET) $(LIBS)

testapp.o: testapp.c
	$(CC) $(CFLAGS) -c testapp.c -o testapp.o

gluethread/glthread.o: gluethread/glthread.c
	$(CC) $(CFLAGS) -c -I gluethread gluethread/glthread.c -o gluethread/glthread.o

graph.o: graph.c
	$(CC) $(CFLAGS) -c -I . graph.c -o graph.o

topologies.o: topologies.c
	$(CC) $(CFLAGS) -c -I . topologies.c -o topologies.o

net.o: net.c
	$(CC) $(CFLAGS) -c -I . net.c -o net.o

CommandParser/libcli.a:
	(cd CommandParser; $(MAKE))

clean:
	rm -f *.o
	rm -f gluethread/glthread.o
	rm -f $(TARGET)
	(cd CommandParser; $(MAKE) clean)

all:
	$(MAKE)
	(cd CommandParser; $(MAKE))
