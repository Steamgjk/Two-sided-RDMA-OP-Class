#include <fcntl.h>
#include <libgen.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <time.h>
#include <vector>
#include <list>
#include <thread>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <fstream>
#include <sys/time.h>
#include <map>
#include "common.h"
#include "messages.h"
using namespace std;
struct client_context
{
	char *buffer;
	size_t buffer_len;
	bool buf_prepared;
	struct ibv_mr *buffer_mr;

	struct message *msg;
	struct ibv_mr *msg_mr;

	uint64_t peer_addr;
	uint32_t peer_rkey;
};
class client_op
{
public:
	client_op();
	client_op(string lip, string rip, int rport);
	static void write_remote(struct rdma_cm_id *id, uint32_t len);
	static void post_receive(struct rdma_cm_id *id);
	static void send_next_chunk(struct rdma_cm_id *id);
	static void on_pre_conn(struct rdma_cm_id *id);
	static void on_completion(struct ibv_wc *wc);
	static void prepare_data(char* buf2send, size_t buf2send_len);
	static void run();
	~client_op();
private:
	static string local_ip;
	static string remote_ip;
	static int remote_port;
	static struct client_context ctx;

};