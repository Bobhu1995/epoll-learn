#ifndef SERVER_FRAMEWORK_H_
#define SERVER_FRAMEWORK_H_


#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ServerTypes.h"
#include "Config.h"

namespace Server {

class ConnectFrame
{
public:
	ConnectFrame();
	virtual ~ConnectFrame() {}

	int32_t init();
	int32_t working();
private:
	//ConnectFrame(const ConnectFrame& conn);
	//ConnectFrame& operator = (const ConnectFrame& conn);
	int32_t load_config();

	int32_t recv_messages();
	
	int32_t recv_messages(int32_t fd);

	int32_t socket_send(int32_t fd, char* data, int32_t length);

	int32_t socket_recv(int32_t fd, char* data, int32_t length);

	void clear_socket(int32_t fd, const char* function = "NULL", int line = 0);
protected:
	//初始化监听epollfd
	int32_t epoll_init(void);

	int32_t open_epoll_socket(uint16_t port, char* local_ip);

	int32_t epoll_new_socket(int32_t fd);

	int32_t epoll_socket(int32_t domain, int32_t type, int32_t protocol);

	void epoll_close(int32_t fd);

	void epoll_destroy();

private:
	//成员变量
	int32_t epoll_fd;
	struct epoll_event* epoll_ptr;
	struct epoll_event epoll_events;

	SockInfo socket_info[MAX_SOCKET_COUNT];
	ServerConfig server_config;

	char msg_buffer[0xfffff];
};

}

#endif //SERVER_FRAMEWORK_H_
