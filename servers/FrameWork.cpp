#include <string.h>
#include <malloc.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <execinfo.h>
#include "FrameWork.h"


namespace Server {
	ConnectFrame::ConnectFrame():epoll_fd(-1), epoll_ptr(NULL){

	}

	ConnectFrame::~ConnectFrame(){

	}

	int32_t ConnectFrame::init() {
		if (FAIL == load_config()) {
			return FAIL;
		}
		
		for (size_t i = 0; i < sizeof(socket_info) / sizeof(SockInfo); ++i) {
			memset(&socket_info[i], 0, sizeof(SockInfo));
			socket_info[i].sockfd = socket_fd_invalid;
		}

		epoll_ptr = NULL;
		for (int32_t i = 0; i < server_config.open_port_count; ++i) {
			open_epoll_socket(server_config.open_prots[i], &server_config.local_ip[0]);
		}

		return SUCCESS;
	}

	int32_t ConnectFrame::load_config() {
		char tmp[] = "127.0.0.1";
		for (size_t i = 0; i < strlen(tmp); ++i) {
			server_config.local_ip[i] = tmp[i];
		}
		server_config.open_port_count = 1;
		server_config.open_prots[0] = 8888;
		server_config.socket_buffer_size = 131072;
		return SUCCESS;
	}

	int32_t ConnectFrame::working() {
		while (true){
			recv_messages();
		}
	}

	int32_t ConnectFrame::epoll_init(){
		if (NULL != epoll_ptr) {
			return FAIL;
		}

		memset(&epoll_events, 0, sizeof(epoll_events));
		epoll_events.events = EPOLLIN | EPOLLERR | EPOLLHUP;
		epoll_events.data.ptr = NULL;
		epoll_events.data.fd = -1;

		epoll_ptr = (struct epoll_event*)malloc(MAX_SOCKET_COUNT * sizeof(struct epoll_event*));
		if (NULL == epoll_ptr) {
			return FAIL;
		}

		epoll_fd = epoll_create(MAX_SOCKET_COUNT);
		if(epoll_fd < 0){
			return FAIL;
		}

		return SUCCESS;
	}

	int32_t ConnectFrame::epoll_socket(int32_t domain, int32_t type, int32_t protocol) {
		int32_t fd = socket(domain, type, protocol);
		if (fd < 0) {
			return FAIL;
		}

		int flags = -1;
		if(ioctl(fd, FIONBIO, &flags) && ((flags = fcntl(fd, F_GETFL, 0)) < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)){
			close(fd);
			return FAIL;
		}

		return fd;
	}

	int32_t ConnectFrame::open_epoll_socket(uint16_t port, char *local_ip) {
		if (epoll_init() != SUCCESS) {
			return FAIL;
		}

		int32_t socket_fd = epoll_socket(AF_INET, SOCK_STREAM, 0);
		if (socket_fd < 0 || socket_fd > MAX_SOCKET_COUNT) {
			epoll_destroy();
			return FAIL;
		}

		int flags = 1;
		struct linger ling = { 0, 0 };
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags));
		setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags));
		setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));

		setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(flags)); //set TCP_CORK

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		if (strlen(local_ip) < 0 || 0 >= inet_pton(AF_INET, local_ip, &(addr.sin_addr))) {
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
		}

		int ret = 0;
		ret = bind(socket_fd, (struct sockaddr*) &addr, sizeof(addr));
		if (ret < 0) {
			epoll_close(socket_fd);
			epoll_destroy();
			return FAIL;
		}

		int32_t opt_value = 0;
		socklen_t opt_len = 0;

		opt_len = (socklen_t)sizeof(opt_value);
		opt_value = 128 * 1024;

		if (0 != setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (const void*)&opt_value, opt_len)) {
			return FAIL;
		}
		if (0 != setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, (const void*)&opt_value, opt_len)) {
			return FAIL;
		}

		ret = listen(socket_fd, 128);
		if (ret < 0) {
			return FAIL;
		}
		
		if (epoll_new_socket(socket_fd) < 0) {
			epoll_close(socket_fd);
			epoll_destroy();
			return FAIL;
		}

		socket_info[socket_fd].sockfd = socket_fd;
		socket_info[socket_fd].create_time = time(NULL);
		socket_info[socket_fd].socket_type = SOCKET_LISTEN;

		return SUCCESS;
	}

	int32_t ConnectFrame::epoll_new_socket(int32_t fd) {
		epoll_events.data.fd = fd;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_events) < 0) {
			return FAIL;
		}

		return SUCCESS;
	}

	void ConnectFrame::epoll_close(int32_t fd) {
		close(fd);
	}

	void ConnectFrame::epoll_destroy() {
		free(epoll_ptr);
		close(epoll_fd);
	}

	void ConnectFrame::clear_socket(int32_t fd, const char* function /* = "NULL" */, int line /* = 0 */) {
		SockInfo* current_sock_ptr = &socket_info[fd];

		int fd1 = current_sock_ptr->sockfd;

		if (fd != fd1) {
			return;
		}

		close(fd1);
		epoll_events.data.fd = fd1;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd1, &epoll_events) < 0) {

		}
		current_sock_ptr->is_sent_message = false;
		current_sock_ptr->create_time = 0;
		current_sock_ptr->recvd_bytes = 0;
		current_sock_ptr->sockfd = socket_fd_invalid;
		current_sock_ptr->uid = 0;

		return;
	}

	int32_t ConnectFrame::recv_messages() {
		int32_t timeout = 1;
		int fd_event_count = 0;

		fd_event_count = epoll_wait(epoll_fd, epoll_ptr, MAX_SOCKET_COUNT, timeout);
		if (fd_event_count < 0){
			return FAIL;
		}

		int sock_fd = socket_fd_invalid;
		struct epoll_event* event_ptr = epoll_ptr;
		SockInfo* current_socket_ptr = NULL;

		for (int i = 0; i < fd_event_count; ++i) {
			sock_fd = event_ptr->data.fd;
			if (sock_fd < 0 || sock_fd >(int)(sizeof(socket_info) / sizeof(socket_info[0]))) {
				continue;
			}

			if(0 != (EPOLLERR & event_ptr->events)){
				clear_socket(sock_fd);
				continue;
			}
			if (0 == (EPOLLIN & event_ptr->events)) {
				continue;
			}

			int accepted_sockfd = socket_fd_invalid;
			struct sockaddr_in socket_address;
			socklen_t socket_address_len = (socklen_t)sizeof(socket_address);

			current_socket_ptr = &socket_info[sock_fd];
			//¼àÌý¶Ë¿Ú
			if (current_socket_ptr->socket_type == SOCKET_LISTEN) {
				accepted_sockfd = accept(sock_fd, (struct sockaddr*) & socket_address, &socket_address_len);
				if (accepted_sockfd <= 0) {
					continue;
				}

				if (accepted_sockfd >= MAX_SOCKET_COUNT) {
					continue;
				}

				int flags = 1;
				if (ioctl(accepted_sockfd, FIONBIO, &flags) && ((flags = fcntl(accepted_sockfd, F_GETFL, 0)) < 0
					|| fcntl(accepted_sockfd, F_SETFL, flags | O_NONBLOCK) < 0)) {
					close(accepted_sockfd);
					continue;
				}

				int ret = 0;
				ret = epoll_new_socket(accepted_sockfd);
				if (ret < 0) {
					clear_socket(accepted_sockfd);
					continue;
				}

				socket_info[accepted_sockfd].sockfd = accepted_sockfd;
				socket_info[accepted_sockfd].is_sent_message = false;
				socket_info[accepted_sockfd].create_time = time(NULL);
				socket_info[accepted_sockfd].uid = 0;
				socket_info[accepted_sockfd].recvd_bytes = 0;
				socket_info[accepted_sockfd].socket_type = SOCKET_TRANSIT;
			}
			else{
				recv_messages(sock_fd);
			}
		}
		return SUCCESS;
	}

	int32_t ConnectFrame::recv_messages(int32_t fd) {
		SockInfo* current_socket_ptr = &socket_info[fd];
		int sock_fd = current_socket_ptr->sockfd;

		if (fd != sock_fd) {
			return FAIL;
		}

		int32_t buffer_size = (int32_t)sizeof(msg_buffer);
		int32_t received = socket_recv(sock_fd, msg_buffer, buffer_size);
		if (received <= 0) {
			clear_socket(sock_fd, __FUNCTION__, __LINE__);
			return FAIL;
		}
		//if (current_socket_ptr->is_sent_message == false) {
		//	char* tmp = msg_buffer[0];

		//	if (0 == *tmp) {
		//		clear_socket(sock_fd);
		//		return FAIL;
		//	}

		//	current_socket_ptr->is_sent_message =
		//}

		printf("%s, message is %s", __FUNCTION__, msg_buffer);
		int32_t sent = socket_send(current_socket_ptr->sockfd, msg_buffer, received);
		if (sent != received) {
			return FAIL;
		}

		return SUCCESS;
	}

	int32_t ConnectFrame::socket_recv(int32_t fd, char* data, int32_t length) {
		if (data == NULL || length < 0) {
			return FAIL;
		}

		size_t received = 0;
		while (true) {
			received = recv(fd, data, length, 0);
			if (received >= 0) {
				return received;
			}
			else {
				if (EAGAIN == errno || EINTR == errno) {
					continue;
				}

				return received;
			}
		}
		return FAIL;
	}

	int32_t ConnectFrame::socket_send(int32_t fd, char* data, int32_t length) {
		if (data == NULL || length < 0) {
			return FAIL;
		}

		int32_t remainded = length;
		int32_t sended = 0;

		char* tmp = data;
		while (remainded > 0){
			sended = send(fd, tmp, (size_t)remainded, 0);
			if (sended > 0) {
				tmp += sended;
				remainded -= sended;
			}
			else {
				if (errno != EINTR && errno != EAGAIN) {
					break;
				}
			}
		}
		return (length - remainded);
	}
}
