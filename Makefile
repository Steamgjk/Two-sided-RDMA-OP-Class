.PHONY: clean
CC=g++
CFLAGS  := -Wall -g -fpermissive -std=c++11
LD      := g++
LDLIBS  := ${LDLIBS} -lrdmacm -libverbs -lpthread
APPS    := client server test_client

all: ${APPS}

test_client: common.o client_op.o test_client.o
	${CC} $(CFLAGS) -o $@ $^ ${LDLIBS}

client: common.o client.o
	${CC} $(CFLAGS) -o $@ $^ ${LDLIBS}

server: common.o server.o
	${CC} $(CFLAGS) -o $@ $^ ${LDLIBS}

clean:
	rm -f *.o ${APPS}

