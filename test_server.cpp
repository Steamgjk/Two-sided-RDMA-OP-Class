#include "rdma_two_sided_server_op.h"
#include <string>
#include <thread>
using namespace std;
struct conn_context ctx, ctx2;
void run(int thread_id);
int main(int argc, char **argv)
{
	printf("waiting for connections. interrupt (^C) to exit.\n");
	ctx.buf_prepared = false;
	ctx2.buf_prepared = false;
	ctx.buf_registered = false;
	ctx2.buf_registered = false;

	std::thread recv_thread(run, 1);
	recv_thread.detach();
	std::thread recv_thread2(run, 2);
	recv_thread2.detach();
	while (1 == 1)
	{
		printf("main thread\n");
		while (ctx.buf_prepared == false || ctx2.buf_prepared == false)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		printf("ctx1 buf =%s  len = %ld\n", ctx.buffer, ctx.buf_len );
		printf("ctx2 buf =%s  len = %ld\n", ctx2.buffer, ctx2.buf_len );
		ctx.buf_prepared = false;
		ctx2.buf_prepared = false;
	}

	return 0;
}
void run(int thread_id)
{
	printf("thread_id=%d\n", thread_id);
	RdmaTwoSidedServerOp rtos;
	if (thread_id == 1)
	{
		rtos.rc_server_loop("44411", &ctx);
	}
	else
	{
		rtos.rc_server_loop("44412", &ctx2);
	}
}