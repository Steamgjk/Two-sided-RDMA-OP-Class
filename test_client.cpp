#include "client.h"
#include <string>
#include <thread>
using namespace std;
struct client_context ctx;
string remote_ip = "12.12.10.16";
void run(int thread_id);
int main(int argc, char **argv)
{

	if (argc >= 2)
	{
		remote_ip = argv[1];
	}

	ctx.buf_prepared = true;
	rc_init(
	    on_pre_conn,
	    NULL, // on connect
	    on_completion,
	    NULL); // on disconnect

	int thread_id = 1;
	std::thread recv_thread(run, thread_id);
	recv_thread.detach();
	while (1 == 1)
	{
		printf("main thread\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		//ctx.buf_prepared = true;
	}

	return 0;
}
void run(int thread_id)
{
	printf("thread_id=%d\n", thread_id);
	rc_client_loop(remote_ip.c_str(), DEFAULT_PORT, &ctx);
}