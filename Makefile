all: server client test_client
CC=g++
TARGET = server
TARGET1 = client
TARGET2 = test_client
LIBS=-libverbs -lrdmacm -pthread -libverbs -lrdmacm
CFLAGS=-Wall -g -fpermissive -std=c++11
OBJS=common.o server.o
OBJS1=common.o client.o
OBJS2=test_client.o common.o client_op.o 

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
$(TARGET1): $(OBJS1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(OBJS1) $(LIBS)
$(TARGET2): $(OBJS2)
	$(CC) $(CFLAGS) -o $(TARGET2) $(OBJS2) $(LIBS)
server.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp
client.o: client.cpp
	$(CC) $(CFLAGS) -c client.cpp
test_client.o: test_client.cpp
	$(CC) $(CFLAGS) -c test_client.cpp
common.o: common.cpp
	$(CC) $(CFLAGS) -c common.cpp
client_op.o: client_op.cpp
	$(CC) $(CFLAGS) -c client_op.cpp

clean:
	rm -rf *.o  *~
