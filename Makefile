all: test_client server
CC=g++
TARGET = test_client
TARGET1 = server
LIBS=-libverbs -lrdmacm -pthread -libverbs -lrdmacm
CFLAGS=-Wall -g -fpermissive -std=c++11
OBJS=test_client.o common.o client.o
OBJS1=common.o server.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
$(TARGET1): $(OBJS1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(OBJS1) $(LIBS)
test_client.o: test_client.cpp
	$(CC) $(CFLAGS) -c test_client.cpp
server.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp
client.o: client.cpp
	$(CC) $(CFLAGS) -c client.cpp
common.o: common.cpp
	$(CC) $(CFLAGS) -c common.cpp
clean:
	rm -rf *.o  *~
