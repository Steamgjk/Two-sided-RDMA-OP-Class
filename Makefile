all: test_client test_server
CC=g++
TARGET = test_client
TARGET1 = test_server
LIBS=-libverbs -lrdmacm -pthread -libverbs -lrdmacm
CFLAGS=-Wall -g -fpermissive -std=c++11
OBJS=test_client.o rdma_two_sided_client_op.o
OBJS1=test_server.o rdma_two_sided_server_op.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
$(TARGET1): $(OBJS1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(OBJS1) $(LIBS)
test_client.o: test_client.cpp
	$(CC) $(CFLAGS) -c test_client.cpp
test_server.o: test_server.cpp
	$(CC) $(CFLAGS) -c test_server.cpp
rdma_two_sided_client_op.o: rdma_two_sided_client_op.cpp
	$(CC) $(CFLAGS) -c rdma_two_sided_client_op.cpp
rdma_two_sided_server_op.o: rdma_two_sided_client_op.cpp
	$(CC) $(CFLAGS) -c rdma_two_sided_server_op.cpp
clean:
	rm -rf *.o  *~
