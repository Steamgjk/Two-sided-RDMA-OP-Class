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
#include <netdb.h>
#include <rdma/rdma_cma.h>
#include "messages.h"
using namespace std;
const int TIMEOUT_IN_MS = 500;

#define TEST_NZ(x) do { if ( (x)) rc_die("error: " #x " failed (returned non-zero)." ); } while (0)
#define TEST_Z(x)  do { if (!(x)) rc_die("error: " #x " failed (returned zero/null)."); } while (0)

typedef void (*pre_conn_cb_fn)(struct rdma_cm_id *id);
typedef void (*connect_cb_fn)(struct rdma_cm_id *id);
typedef void (*completion_cb_fn)(struct ibv_wc *wc);
typedef void (*disconnect_cb_fn)(struct rdma_cm_id *id);

struct context
{
	struct ibv_context *ctx;
	struct ibv_pd *pd;
	struct ibv_cq *cq;
	struct ibv_comp_channel *comp_channel;

	pthread_t cq_poller_thread;
};

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
	void send_next_chunk(struct rdma_cm_id *id);
	static void on_pre_conn(struct rdma_cm_id *id);
	static void on_completion(struct ibv_wc *wc);
	void prepare_data(char* buf2send, size_t buf2send_len);
	void run();


	void build_connection(struct rdma_cm_id *id);
	void build_context(struct ibv_context *verbs);
	void build_params(struct rdma_conn_param *params);
	void build_qp_attr(struct ibv_qp_init_attr *qp_attr);
	void event_loop(struct rdma_event_channel *ec, int exit_on_disconnect);
	void* poll_cq(void *ctx);
	void rc_init(pre_conn_cb_fn pc, connect_cb_fn conn, completion_cb_fn comp, disconnect_cb_fn disc);
	void rc_client_loop(const char *host, const char *port, void *context);
	void rc_server_loop(const char *port);
	void rc_disconnect(struct rdma_cm_id *id);
	void rc_die(const char *reason);
	struct ibv_pd* rc_get_pd();

	~client_op();
private:
	string local_ip;
	string remote_ip;
	int remote_port;
	struct client_context ctx;

	struct context *s_ctx = NULL;
	pre_conn_cb_fn s_on_pre_conn_cb = NULL;
	connect_cb_fn s_on_connect_cb = NULL;
	completion_cb_fn s_on_completion_cb = NULL;
	disconnect_cb_fn s_on_disconnect_cb = NULL;

};