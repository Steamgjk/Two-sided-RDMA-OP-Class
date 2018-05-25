#include "client.h"
#include <string>
#include <thread>
using namespace std;
struct client_context ctx, ctx2;
string remote_ip = "12.12.10.16";
void run(int thread_id);
int main(int argc, char **argv)
{

	if (argc >= 2)
	{
		remote_ip = argv[1];
	}

	ctx.buf_prepared = false;
	ctx.buf_registered = false;

	int thread_id = 1;
	std::thread recv_thread(run, 1);
	recv_thread.detach();
	std::thread recv_thread2(run, 2);
	recv_thread.detach();
	int cnt = 0;
	while (1 == 1)
	{
		printf("main thread\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		if (ctx.buf_registered == false)
		{
			printf("register = false\n");
			continue;
		}
		char str[100];
		memset(str, '\0', 100);
		sprintf(str, "iamok-%d", cnt);
		ctx.buf_len = 100;
		memcpy(ctx.buffer, str, 100);
		ctx.buf_prepared = true;
		cnt++;
	}

	return 0;
}
void run(int thread_id)
{
	printf("thread_id=%d  ctx_ptr=%p  ctx2=%p\n", thread_id, &ctx, &ctx2);
	client ct;
	if (thread_id == 1)
		ct.rc_client_loop("12.12.10.16", DEFAULT_PORT, &ctx);
	else
	{
		ct.rc_client_loop("12.12.10.17", DEFAULT_PORT, &ctx2);
	}
}