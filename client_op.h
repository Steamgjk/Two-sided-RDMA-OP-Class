#include <fcntl.h>
#include <libgen.h>
#include <string>
#include <thread>
#include <unistd.h>
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
	void write_remote(struct rdma_cm_id *id, uint32_t len);
	void post_receive(struct rdma_cm_id *id);
	void send_next_chunk(struct rdma_cm_id *id);
	void on_pre_conn(struct rdma_cm_id *id);
	void on_completion(struct ibv_wc *wc);
	void prepare_data(char* buf2send, size_t buf2send_len);
	void run();
	~client_op();
private:
	string local_ip;
	string remote_ip;
	int remote_port;
	struct client_context ctx;

};