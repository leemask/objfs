
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "plr_socket.h"

int HttpClient::init_server_socket()
{
	sprintf (mServerIP, "%s", "192.168.0.36");
	//sprintf (mServerPage, "%s", "/testJm4");
	sprintf (mServerPage, "%s", "/testJm2");
	mServerPort = 3300;
}

HttpClient::HttpClient()
{
	init_server_socket();
	mSocketFd = this->create_socket(mServerIP, mServerPort, 5000);
}
HttpClient::~HttpClient()
{
	this->destroy_socket(mSocketFd);
}
int HttpClient::create_socket(char *ip_addr, int srv_port, int cli_port)
{
	struct sockaddr_in svr_sockaddr;
	struct sockaddr_in cli_sockaddr;

	int connect_status = 0;
	int on = 1;
	int socket_fd = 0;
    int nodelay = 1;

	if(srv_port == 0 || cli_port == 0)
		return -1;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		plr_sock_debug( "error, socket create \n");
		return -1;
	}

	memset(&svr_sockaddr, 0, sizeof(svr_sockaddr));
	svr_sockaddr.sin_family = AF_INET;
	svr_sockaddr.sin_addr.s_addr = inet_addr(ip_addr);
	svr_sockaddr.sin_port = htons(srv_port);

	memset(&cli_sockaddr, 0, sizeof(cli_sockaddr));
	cli_sockaddr.sin_family = AF_INET;
	cli_sockaddr.sin_port = htons(cli_port);

	if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
	{
		plr_sock_debug( "error, set scoket option \n");
		return -1;
	}

	if (setsockopt (socket_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0)
    {
		plr_sock_debug( "error, set scoket option \n");
		return -1;
    }

#ifdef NON_BLOCK_SOCKET
	// joys,2017.01.09
	struct timeval timeout;

	timeout.tv_sec = 20;
    timeout.tv_usec = 0;

	// set a recive timeout
    if (setsockopt (socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
		plr_sock_debug( "error, set scoket option \n");
		return -1;
    }
	// set a send timeout
	if (setsockopt (socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
		plr_sock_debug( "error, set scoket option \n");
		return -1;
    }
#endif

	bind(socket_fd, (struct sockaddr*)&cli_sockaddr, sizeof(struct sockaddr));

	plr_sock_debug( "connect... sockfd : %d, ip : %s, server port : %d client port : %d \n",socket_fd, ip_addr, srv_port, cli_port);
	connect_status = connect(socket_fd, (struct sockaddr*)&svr_sockaddr, sizeof(svr_sockaddr));
	if(connect_status < 0)
	{
		plr_sock_debug( "connect error \n");
		return -1;
	}
	else 
		plr_sock_debug( "connect... complete \n");

	return socket_fd;
}


void HttpClient::destroy_socket(int sock_fd)
{
	int ret = 0;
	if(sock_fd > 0)
	{
 		ret = close(sock_fd);
		if (ret)
			perror("destroy_socket failed");
		plr_sock_debug("close socket \n");
	}
	else
	{
		plr_sock_debug("already close socket \n");
	}
	mSocketFd = 0;
}

//int write_sock(int sockfd, char *buf, int length)
int HttpClient::write_sock(int sockfd, char *buf)
{
	int ret = 0;
	int retry_cnt = RW_RETRY;
	int length = 0;
	char buffer[256] = {0,};

	sprintf(buffer, "%s\n", buf);
	length = strlen(buffer);

	if(sockfd < 0)
	{
		plr_sock_debug("can't find socket descriptor \n");
		//assert(0);
		return -1;
	}

	while(retry_cnt--)
	{
#ifdef NON_BLOCK_SOCKET		
		if(send(sockfd, buffer, length, MSG_NOSIGNAL) != length)
#else
		ret = write(sockfd, buffer, length);
		//if(write(sockfd, buf, length) != length)
		if(ret != length)
#endif			
		{
			plr_sock_debug("%s \n", buffer);
			plr_sock_debug("[%s] retry : %d\n", __func__, retry_cnt);
			sleep(1);
			continue;
		}
		return 0;
	}
	
	printf ("[%s][%d]\n", __func__, __LINE__);
	destroy_socket(sockfd);

	return -1;
}

int HttpClient::read_sock(int sockfd, char *buf, int length)
{
	int ret = 0;
	int retry_cnt = RW_RETRY;

	if(sockfd < 0)
	{
		plr_sock_debug("can't find socket descriptor \n");
		//assert(0);
		return -1;
	}

	while(retry_cnt--){
#ifdef NON_BLOCK_SOCKET		
		ret = recv(sockfd, buf, length, 0);
#else
		ret = read(sockfd, buf, length);
#endif
		if(ret < 0)
		{	
			plr_sock_debug("error, length is negative \n");
			plr_sock_debug("[%s] retry : %d\n", __func__, retry_cnt);
			sleep(1);
			continue;
		}
		else
		{
			plr_sock_debug("[%s]\n", __func__);
			return ret;
		}
		
	}

	printf ("[%s][%d]\n", __func__, __LINE__);
	destroy_socket(sockfd);
	return -1;
}

int HttpClient::SendDataToServer(uint32 data)
{
	char sending_string[128];
	char out_buffer[256] = {0,};

	//mSocketFd = this->create_socket(mServerIP, mServerPort, 5000);

  	sprintf(sending_string, "{\"data\":%d}", data);
	
    sprintf(out_buffer, "POST %s HTTP/1.1", mServerPage);
    this->write_sock(mSocketFd, out_buffer);

    this->write_sock(mSocketFd, (char*)"Host: XXX");
    this->write_sock(mSocketFd, (char*)"User-Agent: Arduino/1.0");
    this->write_sock(mSocketFd, (char*)"Content-Type:application/json");

    sprintf(out_buffer, "Content-Length: %zd", strlen(sending_string));
    this->write_sock(mSocketFd, out_buffer);
    this->write_sock(mSocketFd, (char*)"");
    this->write_sock(mSocketFd, sending_string);
    //this->write_sock(mSocketFd, (char*)"Connection:close");

	//this->destroy_socket(mSocketFd);
	sleep(1);

	return 0;
}
