#include "tcpServer.h"

#include <cstdio>
#include <cstring>

#include <sys/ioctl.h>

#include <unistd.h>

#define BUF_SIZE 4096
#define TCP_TEST_PORT 	7654
#define LOCALHOST 	"127.0.0.1"

#define MAXPENDING 5

void* thread_waitingConnect(void* pData)
{
	CTCPServer* instance = CTCPServer::getInstance() ;
	instance->waitingConnect() ;

	return NULL ;
}

CTCPServer* CTCPServer::getInstance()
{
	static CTCPServer instance ;
	return &instance ;
}

int CTCPServer::setMaxFD(int newFD)
{
	m_fdMax = (newFD > m_fdMax) ? newFD : m_fdMax ;

	return m_fdMax ;
}

int CTCPServer::init()
{
	m_flagDestroy = 0 ;

	m_serverSocket = -1 ;
	m_clientSocket = -1 ;

	m_fdMax = -1 ;

	unsigned short serverPort = TCP_TEST_PORT ;
	sockaddr_in serverAddr ;

	m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ;

	if(m_serverSocket < 0)
	{
		printf("ERROR : cannot create socket~!! (%d)\n", __LINE__) ;
	}

	memset(&serverAddr, 0x0, sizeof(sockaddr_in)) ;

	serverAddr.sin_family 		= AF_INET ;
	serverAddr.sin_addr.s_addr 	= htonl(INADDR_ANY) ;
	serverAddr.sin_port 		= htons(serverPort) ;

	int ret ;
	int option = 1 ;
	setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) ;

	ret = bind(m_serverSocket, (struct sockaddr*)&serverAddr, sizeof(sockaddr_in)) ;
	if(ret < 0)
	{
		printf("ERROR : Server bind failed (%d)\n", __LINE__) ;
	}

	ret = listen(m_serverSocket, MAXPENDING) ;

	// create pipes. The pipe will be used to wake up blocked select().
	pipe(m_pipe) ;

	FD_ZERO(&m_fds) ;
	FD_SET(m_serverSocket, &m_fds) ;
	FD_SET(m_pipe[0], &m_fds) ;

	setMaxFD(m_serverSocket) ;
	setMaxFD(m_pipe[0]) ;
	
	if(ret < 0)
	{
		printf("ERROR : Server listen failed (%d)\n", __LINE__) ;
	}

	pthread_create(&m_threadConnect, NULL, &thread_waitingConnect, NULL) ;

	return 1 ;
}

int CTCPServer::destroy()
{
	int ret ;
	m_flagDestroy = 1 ;

	printf("call server destroy()\n") ;

	// away server thread.
	write(m_pipe[1], &ret, 1) ;

	void* nStatus ;
	pthread_join(m_threadConnect, &nStatus) ;

	close(m_pipe[0]) ;
	close(m_pipe[1]) ;

	if(m_serverSocket >= 0)
	{
		ret = close(m_serverSocket) ;
		printf("ret1 : %d\n", ret) ;
	}

	if(m_clientSocket >= 0)
	{
		ret = close(m_clientSocket) ;
		printf("ret2 : %d\n", ret) ;
	}

	m_flagDestroy = 0 ;

	return 1 ;
}

int CTCPServer::sendData()
{
	char szBuf[128] ;
	int len ;
	strcpy(szBuf, "Server -> Client") ;
	len = strlen(szBuf) ;
	send(m_clientSocket, szBuf, len, 0) ;

	return 1 ;
}

int CTCPServer::waitingConnect()
{
	char szBuf[BUF_SIZE] ;

	int fd ;
	int ret ;
	unsigned int clientLen ;

	fd_set 	checkFds ;

	int nread ;

	int flagAccept = 0 ;

	const unsigned char SERVER_RECEIVES_CONNECTION_REQUEST 	= 1 ;
	const unsigned char SERVER_RECEIVES_DATA 		= 2 ;
	const unsigned char SERVER_CLOSES_CLIENT_CONNECTION 	= 3 ;
	unsigned char flagStatus = 0 ;

	sockaddr_in clientAddr ;

	while(1)
	{
		usleep(1000) ;

		if(m_flagDestroy)
			break ;

		checkFds = m_fds ;

		ret = select(m_fdMax + 1, &checkFds, 0, 0, NULL) ;

		if(ret < 0)
		{
			printf("ERROR : select (%d)\n", __LINE__) ;
		}

		for(fd = 0; fd <= m_fdMax ; fd++)
		{
			if(!FD_ISSET(fd, &checkFds))
				continue ;

			if(FD_ISSET(m_pipe[0], &checkFds))
				break ;

			if(fd == m_serverSocket)
			{
				flagStatus = SERVER_RECEIVES_CONNECTION_REQUEST ; 
			}
			else
			{
				ioctl(fd, FIONREAD, &nread) ;
				if(nread == 0)
					flagStatus = SERVER_CLOSES_CLIENT_CONNECTION ;
				else
					flagStatus = SERVER_RECEIVES_DATA ;
			}
			
			switch(flagStatus)
			{
			case SERVER_RECEIVES_CONNECTION_REQUEST :
				printf("SERVER_RECEIVES_CONNECTION_REQUEST\n") ;
				m_clientSocket = accept(fd, (struct sockaddr*)&clientAddr, (socklen_t*)&clientLen) ;

				if(m_clientSocket < 0)
				{
					printf("accept : %d\n", m_clientSocket) ;
				}
				
				printf("After Accept & m_clientSocket : %d\n", m_clientSocket) ;

				FD_SET(m_clientSocket, &m_fds) ;
				setMaxFD(m_clientSocket) ;
				break ;

			case SERVER_RECEIVES_DATA :
				printf("SERVER_RECEIVES_DATA\n") ;
#if 0
				// 조건에 의해 client의 연결을 거부하는 경우,
				if([condition])
				{
					FD_CLR(fd, &m_readfds) ;
					clese(fd) ;
					break ;
				}
#endif
				nread = recv(fd, szBuf, BUF_SIZE, 0) ;
				printf("Received Data : %s, nread : %d\n", szBuf, nread) ;

				if(nread == 1)
				{
					send(fd, szBuf, 1, 0) ;
					break ;
				}

				if(nread > 0)
				{
//					parseRecvData(szBuf, nread) ;
					;
				}
				break ;
			case SERVER_CLOSES_CLIENT_CONNECTION :
				printf("SERVER_CLOSES_CLIENT_CONNECTION\n") ;
				FD_CLR(fd, &m_fds) ;
				close(fd) ;

				printf("close FD by client, fd : %d\n", fd) ;
				break ;
			}
		}
	}

	return 1 ;
}
