.PHONY: clean

CFLAGS  := -Wall -Werror -g -fpermissive -std=c++11
LD      := g++
LDLIBS  := ${LDLIBS} -lrdmacm -libverbs -lpthread
APPS    := client server

all: ${APPS}

client: common.o client.o
	${LD} -o $@ $^ ${LDLIBS}

server: common.o server.o
	${LD} -o $@ $^ ${LDLIBS}

clean:
	rm -f *.o ${APPS}

