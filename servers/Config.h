#ifndef SERVER_CONFIG_H_
#define SERVER_CONFIG_H_
#include "ServerTypes.h"

namespace Server{
#define MAX_SOCKET_COUNT 1024

enum
{
	FAIL = -1,
	SUCCESS = 0,
};

enum 
{
	SOCKET_LISTEN = 0,
	SOCKET_TRANSIT = 1,
};

enum
{
	max_open_port_count = 8,			//服务器打开的端口最大数目

#if defined(__DEBUG__) || defined(_DEBUG_)
	max_socket_count = 0x0ff0,          //支持的最大连接数
#else
	max_socket_count = 0xf000,          //支持的最大连接数
#endif


	max_ip_address_length = 30,			//

	auxiliary_handler_count = 2,		//

	max_s2c_package_size = 0xfffff,		//service server发送给前端最大的包大小

	max_package_count_per_sending = 512,//		

	max_file_path = 255,				//

	socket_fd_invalid = -1,				//

	max_policy_file_size = 10 * 1024,      //10K空间 存policy file
};

struct SockInfo
{
	int32_t uid;
	int32_t sockfd;
	int32_t socket_type;
	bool is_sent_message;
	int32_t recvd_bytes;

	uint64_t create_time;
};

struct ServerConfig
{
	int32_t open_port_count;
	uint16_t open_prots[max_open_port_count];
	int32_t socket_buffer_size;
	
	char local_ip[max_ip_address_length];
};

}


#endif //SERVER_CONFIG_H_
