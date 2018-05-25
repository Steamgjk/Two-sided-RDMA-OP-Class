#include "rdma_two_sided_server_op.h"
#include <string>
#include <thread>
using namespace std;
int main(int argc, char **argv)
{
	/*
	rc_init(
	    on_pre_conn,
	    on_connection,
	    on_completion,
	    on_disconnect);
	    **/
	RdmaTwoSidedServerOp rtos;
	printf("waiting for connections. interrupt (^C) to exit.\n");

	rtos.rc_server_loop(DEFAULT_PORT);

	return 0;
}