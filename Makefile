.PHONY: clean
CC=g++
CFLAGS  := -Wall -Werror -g -fpermissive -std=c++11
LD      := g++
LDLIBS  := ${LDLIBS} -lrdmacm -libverbs -lpthread
APPS    := client server

all: ${APPS}

client: common.o client.o
	${CC} -o $@ $^ ${LDLIBS}

server: common.o server.o
	${CC} -o $@ $^ ${LDLIBS}

clean:
	rm -f *.o ${APPS}

