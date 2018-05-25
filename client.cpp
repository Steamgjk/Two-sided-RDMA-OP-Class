#include "client.h"




void client_write_remote(struct rdma_cm_id *id, uint32_t len)
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

void client_post_receive(struct rdma_cm_id *id)
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

void client_send_next_chunk(struct rdma_cm_id *id)
{
  struct client_context *ctx = (struct client_context *)id->context;
  while (!ctx->buf_prepared)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }
  /*
  char*str = "iamok";
  ctx->buf_len = strlen(str);
  memcpy(ctx->buffer, str, ctx->buf_len);
  **/
  printf("send tchunk...\n");
  printf("buf= %s  len=%ld\n", ctx->buffer, ctx->buf_len );

  write_remote(id, ctx->buf_len);
  ctx->buf_prepared = false;
}



void client_on_pre_conn(struct rdma_cm_id *id)
{
  struct client_context *ctx = (struct client_context *)id->context;
  posix_memalign((void **)&ctx->buffer, sysconf(_SC_PAGESIZE), BUFFER_SIZE);
  TEST_Z(ctx->buffer_mr = ibv_reg_mr(rc_get_pd(), ctx->buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE));

  posix_memalign((void **)&ctx->msg, sysconf(_SC_PAGESIZE), sizeof(*ctx->msg));
  TEST_Z(ctx->msg_mr = ibv_reg_mr(rc_get_pd(), ctx->msg, sizeof(*ctx->msg), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE));


  ctx->buf_registered = true;
  printf("ok registered  ptr=%p\n", ctx );
  client_post_receive(id);
}

void client_on_completion(struct ibv_wc *wc)
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
      client_send_next_chunk(id);
    }
    else if (ctx->msg->id == MSG_READY)
    {
      printf("received READY, sending chunk\n");
      client_send_next_chunk(id);
    }
    else if (ctx->msg->id == MSG_DONE)
    {
      printf("received DONE, disconnecting\n");
      rc_disconnect(id);
      return;
    }

    client_post_receive(id);
  }
}


