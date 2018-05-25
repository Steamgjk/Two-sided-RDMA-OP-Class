#include "client_op.h"
#include <thread>
using namespace std;
client_op*c_op = NULL;
void rdma_sendTd(int send_thread);
int main()
{
	c_op = new client_op("12.12.10.16", 12345, "12.12.10.18");
	int thid = 1;
	std::thread send_thread(rdma_sendTd, thid);
	send_thread.detach();
	while (1 == 1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		char* str = "Iamok";
		size_t len = strlen(str);
		c_op->prepare_data(str, len);
		printf("prepared!\n");
	}


}
void rdma_sendTd(int send_thread)
{
	c_op->run();
}