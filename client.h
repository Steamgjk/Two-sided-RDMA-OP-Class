#ifndef RDMA_CLIENT_H
#define RDMA_CLIENT_H

#include <fcntl.h>
#include <libgen.h>
#include <string>
#include <thread>
#include "common.h"
#include "messages.h"
using namespace std;

struct client_context
{
	char *buffer;
	size_t buf_len;
	bool buf_prepared;
	bool buf_registered;
	struct ibv_mr *buffer_mr;

	struct message *msg;
	struct ibv_mr *msg_mr;

	uint64_t peer_addr;
	uint32_t peer_rkey;
};
void client_write_remote(struct rdma_cm_id *id, uint32_t len);
void client_post_receive(struct rdma_cm_id *id);
void client_send_next_chunk(struct rdma_cm_id *id);
void client_on_pre_conn(struct rdma_cm_id *id);
void client_on_completion(struct ibv_wc *wc);
void client_event_loop(struct rdma_event_channel *ec, int exit_on_disconnect);

void rc_client_loop(const char *host, const char *port, void *context);

#endif