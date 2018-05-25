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

  client_write_remote(id, ctx->buf_len);
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



void rc_client_loop(const char *host, const char *port, void *context)
{
  struct addrinfo *addr;
  struct rdma_cm_id *conn = NULL;
  struct rdma_event_channel *ec = NULL;
  struct rdma_conn_param cm_params;

  TEST_NZ(getaddrinfo(host, port, NULL, &addr));

  TEST_Z(ec = rdma_create_event_channel());
  TEST_NZ(rdma_create_id(ec, &conn, NULL, RDMA_PS_TCP));
  TEST_NZ(rdma_resolve_addr(conn, NULL, addr->ai_addr, TIMEOUT_IN_MS));

  freeaddrinfo(addr);

  conn->context = context;

  build_params(&cm_params);

  client_event_loop(ec, 1); // exit on disconnect

  rdma_destroy_event_channel(ec);
}



void client_event_loop(struct rdma_event_channel *ec, int exit_on_disconnect)
{
  struct rdma_cm_event *event = NULL;
  struct rdma_conn_param cm_params;

  build_params(&cm_params);

  while (rdma_get_cm_event(ec, &event) == 0)
  {
    struct rdma_cm_event event_copy;

    memcpy(&event_copy, event, sizeof(*event));
    rdma_ack_cm_event(event);

    if (event_copy.event == RDMA_CM_EVENT_ADDR_RESOLVED)
    {
      build_connection(event_copy.id);
      /*
            if (s_on_pre_conn_cb)
              s_on_pre_conn_cb(event_copy.id);
      **/
      client_on_pre_conn(event_copy.id);

      TEST_NZ(rdma_resolve_route(event_copy.id, TIMEOUT_IN_MS));

    }
    else if (event_copy.event == RDMA_CM_EVENT_ROUTE_RESOLVED)
    {
      TEST_NZ(rdma_connect(event_copy.id, &cm_params));

    }
    else if (event_copy.event == RDMA_CM_EVENT_CONNECT_REQUEST)
    {
      build_connection(event_copy.id);

      client_on_pre_conn(event_copy.id);
      /*
      if (s_on_pre_conn_cb)
        s_on_pre_conn_cb(event_copy.id);
      **/
      TEST_NZ(rdma_accept(event_copy.id, &cm_params));

    }
    else if (event_copy.event == RDMA_CM_EVENT_ESTABLISHED)
    {
      /*
      if (s_on_connect_cb)
        s_on_connect_cb(event_copy.id);
        **/


    }
    else if (event_copy.event == RDMA_CM_EVENT_DISCONNECTED)
    {
      rdma_destroy_qp(event_copy.id);

      /*
      if (s_on_disconnect_cb)
        s_on_disconnect_cb(event_copy.id);
      **/
      rdma_destroy_id(event_copy.id);

      if (exit_on_disconnect)
        break;

    }
    else
    {
      rc_die("unknown event\n");
    }
  }
}