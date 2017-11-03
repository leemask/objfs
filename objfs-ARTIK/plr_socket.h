#ifndef __PLR_SOCKET__
#define __PLR_SOCKET__

#include <fat_types.h>

// joys,2017.01.09
//#define NON_BLOCK_SOCKET

#define CONNECT_RETRY			3
#define RW_RETRY				3

// #define	O__USE_PLR_SOCK_DEBUG
#define	O__USE_PLR_SOCK_DEBUG
#ifdef	O__USE_PLR_SOCK_DEBUG

#define plr_sock_debug(fmt, args...)	printf("[L%d][%s] " fmt, __LINE__, __func__, ##args)
#else
#define plr_sock_debug(fmt, args...)	
#endif

class HttpClient
{
private:
	int mSocketFd;
	char mServerIP[32];
	char mServerPage[32];
	int mServerPort;

	int init_server_socket();

public:
	HttpClient();
	~HttpClient();

	int create_socket(char *ip_addr, int srv_port, int cli_port);
	void destroy_socket(int sock_fd);
	int write_sock(int sockfd, char *buf);
	int read_sock(int sockfd, char *buf, int length);
	int SendDataToServer(uint32 data);
};

#endif

