#include "client_op.h"

client_op::client_op()
{
}
client_op::client_op(string lip, string rip, int rport)
{
	remote_ip = rip;
	remote_port = rport;
	local_ip = lip;

}
client_op::~client_op()
{
}
void client_op::write_remote(struct rdma_cm_id *id, uint32_t len)
{
	struct client_context *ctx = (struct client_context *)id->context;

	struct ibv_send_wr wr, *bad_wr = NULL;
	struct ibv_sge sge;

	memset(&wr, 0, sizeof(wr));

	wr.wr_id = (uintptr_t)id;
	wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
	wr.send_flags = IBV_SEND_SIGNALED;
	wr.imm_data = htonl(len);
	wr.wr.rdma.remote_addr = ctx->peer_addr;
	wr.wr.rdma.rkey = ctx->peer_rkey;

	if (len)
	{
		wr.sg_list = &sge;
		wr.num_sge = 1;

		sge.addr = (uintptr_t)ctx->buffer;
		sge.length = len;
		sge.lkey = ctx->buffer_mr->lkey;
	}

	TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));
}

void client_op::post_receive(struct rdma_cm_id *id)
{
	struct client_context *ctx = (struct client_context *)id->context;

	struct ibv_recv_wr wr, *bad_wr = NULL;
	struct ibv_sge sge;

	memset(&wr, 0, sizeof(wr));

	wr.wr_id = (uintptr_t)id;
	wr.sg_list = &sge;
	wr.num_sge = 1;

	sge.addr = (uintptr_t)ctx->msg;
	sge.length = sizeof(*ctx->msg);
	sge.lkey = ctx->msg_mr->lkey;

	TEST_NZ(ibv_post_recv(id->qp, &wr, &bad_wr));
}

void client_op::send_next_chunk(struct rdma_cm_id *id)
{
	printf("come to send_chunk\n");
	struct client_context *ctx = (struct client_context *)id->context;
	while (!ctx->buf_prepared)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}

	printf("send tchunk...\n");
	printf("buf= %s\n", ctx->buffer );
	write_remote(id, ctx->buffer_len);
	ctx->buf_prepared = false;
}



void client_op::on_pre_conn(struct rdma_cm_id *id)
{
	struct client_context *ctx = (struct client_context *)id->context;
	posix_memalign((void **)&ctx->buffer, sysconf(_SC_PAGESIZE), BUFFER_SIZE);
	TEST_Z(ctx->buffer_mr = ibv_reg_mr(rc_get_pd(), ctx->buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE));

	posix_memalign((void **)&ctx->msg, sysconf(_SC_PAGESIZE), sizeof(*ctx->msg));
	TEST_Z(ctx->msg_mr = ibv_reg_mr(rc_get_pd(), ctx->msg, sizeof(*ctx->msg), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));

	post_receive(id);
}

void client_op::on_completion(struct ibv_wc *wc)
{
	struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)(wc->wr_id);
	struct client_context *ctx = (struct client_context *)id->context;
	if (wc->opcode & IBV_WC_RECV)
	{
		if (ctx->msg->id == MSG_MR)
		{
			ctx->peer_addr = ctx->msg->data.mr.addr;
			ctx->peer_rkey = ctx->msg->data.mr.rkey;

			printf("received MR, sending file name(obsolete), send chunk\n");
			//send_file_name(id);
			send_next_chunk(id);
		}
		else if (ctx->msg->id == MSG_READY)
		{
			printf("received READY, sending chunk\n");
			send_next_chunk(id);
		}
		else if (ctx->msg->id == MSG_DONE)
		{
			printf("received DONE, disconnecting\n");
			rc_disconnect(id);
			return;
		}

		post_receive(id);
	}
}
void client_op::run()
{
	ctx.buf_prepared = false;
	rc_init(
	    this->on_pre_conn,
	    NULL, // on connect
	    this->on_completion,
	    NULL); // on disconnect
	char rport[30];
	//itoa(remote_port, rport, 10);
	snprintf(rport, 30, "%d", remote_port);
	printf("rport=%s\n", rport );
	rc_client_loop(remote_ip.c_str(), rport, &ctx);
}
void client_op::prepare_data(char* buf2send, size_t buf2send_len)
{
	(ctx).buffer_len = buf2send_len;
	memcpy((ctx).buffer, buf2send, buf2send_len);
	(ctx).buf_prepared = true;
}