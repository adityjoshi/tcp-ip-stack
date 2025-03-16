CC=gcc
CFLAGS=-g
TARGET:test.exe CommandParser/libcli.a 
LIBS=-lpthread -L ./CommandParser -lcli
OBJS=gluethread/glthread.o \
		  graph.o 		   \
		  topologies.o	   \
		  net.o			   \
		  layer2/layer2.o  \
		  nwcli.o		   \
		  utils.o		   \
		  layer2/l2switch.o \
		  communication.o
		  

test.exe:testapp.o ${OBJS} CommandParser/libcli.a
	${CC} ${CFLAGS} testapp.o ${OBJS} -o test.exe ${LIBS}

testapp.o:testapp.c
	${CC} ${CFLAGS} -c testapp.c -o testapp.o

gluethread/glthread.o:gluethread/glthread.c
	${CC} ${CFLAGS} -c -I gluethread gluethread/glthread.c -o gluethread/glthread.o

graph.o:graph.c
	${CC} ${CFLAGS} -c -I . graph.c -o graph.o

topologies.o:topologies.c
	${CC} ${CFLAGS} -c -I . topologies.c -o topologies.o

net.o:net.c
	${CC} ${CFLAGS} -c -I . net.c -o net.o



nwcli.o:nwcli.c
	${CC} ${CFLAGS} -c -I . nwcli.c  -o nwcli.o

utils.o:utils.c
	${CC} ${CFLAGS} -c -I . utils.c -o utils.o

communication.o:communication.c
	${CC} ${CFLAGS} -c -I . communication.c -o communication.o
	
layer2/layer2.o:layer2/layer2.c
	${CC} ${CFLAGS} -c -I . layer2/layer2.c -o layer2/layer2.o

layer2/l2switch.o:layer2/l2switch.c
	${CC} ${CFLAGS} -c -I . layer2/l2switch.c -o layer2/l2switch.o

CommandParser/libcli.a:
	(cd CommandParser; make)
clean:
	rm -f *.o
	rm -f gluethread/glthread.o
	rm -f layer2/*.o
	rm -f *exe
	
all:
	make

cleanall:
	make clean
	(cd CommandParser; make clean)
	
