#ifndef RDMA_CLIENT_H
#define RDMA_CLIENT_H

#include <fcntl.h>
#include <libgen.h>
#include <string>
#include <thread>
#include "common.h"
#include "messages.h"
using namespace std;

char* buf2send;
size_t buf2send_len;
struct client_context
{
	char *buffer;
	size_t buf_len;
	bool buf_prepared;
	struct ibv_mr *buffer_mr;

	struct message *msg;
	struct ibv_mr *msg_mr;

	uint64_t peer_addr;
	uint32_t peer_rkey;
};
void write_remote(struct rdma_cm_id *id, uint32_t len);
void post_receive(struct rdma_cm_id *id);
void send_next_chunk(struct rdma_cm_id *id);
void on_pre_conn(struct rdma_cm_id *id);
void on_completion(struct ibv_wc *wc);

#endif