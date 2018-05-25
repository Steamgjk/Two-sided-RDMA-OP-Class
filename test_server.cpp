#include "rdma_two_sided_server_op.h"
#include <string>
#include <thread>
using namespace std;
void run(int thread_id);
int main(int argc, char **argv)
{
	printf("waiting for connections. interrupt (^C) to exit.\n");
	std::thread recv_thread(run, 1);
	recv_thread.detach();
	std::thread recv_thread2(run, 2);
	recv_thread2.detach();
	while (1 == 1)
	{
		printf("main thread\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return 0;
}
void run(int thread_id)
{
	printf("thread_id=%d\n", thread_id);
	RdmaTwoSidedServerOp rtos;
	if (thread_id == 1)
	{
		rtos.rc_server_loop(44411);
	}
	else
	{
		//rtos.rc_server_loop(44412);
	}
}