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
  struct ibv_mr *buffer_mr;

  struct message *msg;
  struct ibv_mr *msg_mr;

  uint64_t peer_addr;
  uint32_t peer_rkey;
};


static void write_remote(struct rdma_cm_id *id, uint32_t len)
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

static void post_receive(struct rdma_cm_id *id)
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

static void send_next_chunk(struct rdma_cm_id *id)
{
  struct client_context *ctx = (struct client_context *)id->context;
  while (!ctx->buf_prepared)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }
  printf("send tchunk...\n");
  printf("buf= %s\n", ctx->buffer );
  getchar();
  write_remote(id, ctx->buf_len);
}



static void on_pre_conn(struct rdma_cm_id *id)
{
  struct client_context *ctx = (struct client_context *)id->context;
  printf("hehre in on_pre_conn  ctx.buf=%p  %s\n", ctx->buffer, ctx->buffer );
  posix_memalign((void **)&ctx->buffer, sysconf(_SC_PAGESIZE), BUFFER_SIZE);
  TEST_Z(ctx->buffer_mr = ibv_reg_mr(rc_get_pd(), ctx->buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE));

  posix_memalign((void **)&ctx->msg, sysconf(_SC_PAGESIZE), sizeof(*ctx->msg));
  TEST_Z(ctx->msg_mr = ibv_reg_mr(rc_get_pd(), ctx->msg, sizeof(*ctx->msg), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));

  post_receive(id);
}

static void on_completion(struct ibv_wc *wc)
{
  struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)(wc->wr_id);
  struct client_context *ctx = (struct client_context *)id->context;
  printf("here in on_completion  ctx->buffer=%p  %s\n", ctx->buffer, ctx->buffer );
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

int main(int argc, char **argv)
{
  struct client_context ctx;

  string remote_ip = "12.12.10.16";
  string file_name = "testfile";
  if (argc >= 2)
  {
    remote_ip = argv[1];
  }
  if (argc >= 3)
  {
    file_name = argv[2];
  }
  char* str = "testok";
  ctx.buf_len = strlen(str);
  ctx.buffer = (char*) malloc(ctx.buf_len);
  memcpy(ctx.buffer, str, ctx.buf_len);
  ctx.buf_prepared = true;
  printf("ctx.buf=%p  %s\n", ctx.buffer, ctx.buffer );
  rc_init(
    on_pre_conn,
    NULL, // on connect
    on_completion,
    NULL); // on disconnect

  rc_client_loop(remote_ip.c_str(), DEFAULT_PORT, &ctx);


  return 0;
}

