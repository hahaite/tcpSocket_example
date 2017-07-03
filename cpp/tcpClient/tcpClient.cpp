#include "tcpClient.h"

#include <cstdio>
#include <unistd.h>
#include <cstring>

#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <errno.h>

#define BUF_SIZE 4096
#define TCP_PORT 7654

#define LOCALHOST 	"127.0.0.1"

#define SERVER_IP 	LOCALHOST
//#define SERVER_IP 	"10.0.0.8"

static void* thread_recv(void* pData)
{
	CTCPClient* instance = CTCPClient::getInstance() ;
	instance->waitingRecv() ;
	return NULL ;
}


CTCPClient* CTCPClient::getInstance()
{
	static CTCPClient instance ;
	return &instance ;
}

int CTCPClient::setMaxFD(int newFD)
{
	m_fdMax = (newFD > m_fdMax) ? newFD : m_fdMax ;
	return m_fdMax ;
}

int CTCPClient::init()
{
	m_flagDestroy = 0 ;
	m_clientSocket = -1 ;

	m_fdMax = -1 ;
	FD_ZERO(&m_fds) ;

	pipe(m_pipe) ;

	///////////////
#if 0
	// to check error number
	printf("EACCES : %d\n", EACCES) ; 
	printf("EADDRINUSE : %d\n", EADDRINUSE) ; 
	printf("EAFNOSUPPORT : %d\n", EAFNOSUPPORT) ; 
	printf("EAGAIN : %d\n", EAGAIN) ; 
	printf("EALREADY : %d\n", EALREADY) ; 
	printf("EBADF : %d\n", EBADF) ; 
	printf("ECONNREFUSED : %d\n", ECONNREFUSED) ; 
	printf("EFAULT : %d\n", EFAULT) ; 
	printf("EINPROGRESS : %d\n", EINPROGRESS) ; 
	printf("EINTR : %d\n", EINTR) ; 
	printf("EISCONN : %d\n", EISCONN) ; 
	printf("ENETUNREACH : %d\n", ENETUNREACH) ; 
	printf("ENOTSOCK : %d\n", ENOTSOCK) ; 
	printf("ETIMEDOUT : %d\n", ETIMEDOUT) ; 
#endif
	///////////////

	pthread_mutex_init(&m_mutex, NULL) ;

	pthread_create(&m_threadRecv, NULL, &thread_recv, NULL) ; 

	return 1 ;
}


int CTCPClient::initSocket()
{
	// initialize socket
	if(m_clientSocket >= 0)
	{
		close(m_clientSocket) ;
		FD_CLR(m_clientSocket, &m_fds) ;
	}

	m_clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP) ;
	if(m_clientSocket < 0)
	{
		printf("ERROR : Failed create socket\n") ;
	}

	FD_SET(m_clientSocket, &m_fds) ;
	FD_SET(m_pipe[0], &m_fds) ;

	setMaxFD(m_clientSocket) ;
	setMaxFD(m_pipe[0]) ;

	int option = 1 ;
	setsockopt(m_clientSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) ;

	return 1 ;
}

int CTCPClient::destroy()
{
	m_flagDestroy = 1 ;

	// wake up select()
	write(m_pipe[1], &m_flagDestroy, 1) ;

	void* nStatus ;
	pthread_join(m_threadRecv, &nStatus) ;

	close(m_pipe[0]) ;
	close(m_pipe[1]) ;

	if(m_clientSocket >= 0)
		close(m_clientSocket) ;

	m_flagDestroy = 0 ;

	printf("Bye Bye~!!\n") ;

	return 1 ;
}

int CTCPClient::sendData()
{
	pthread_mutex_lock(&m_mutex) ;

	char szBuf[128] ;
	int len ;
	strcpy(szBuf, "Client -> Server") ;
	len = strlen(szBuf) ;
	send(m_clientSocket, szBuf, len, 0) ;

	pthread_mutex_unlock(&m_mutex) ;

	return 1 ;
}


int CTCPClient::waitingRecv()
{

	const unsigned char INITIALIZE_SOCKET 		= 1 ;
	const unsigned char CLIENT_IS_TRYING_CONNECT 	= 2 ;
	const unsigned char CLIENT_IS_WAITING 		= 3 ;
	const unsigned char CLIENT_RECEIVED_DATA 	= 4 ;
	const unsigned char SERVER_CLOSES_CONNECTION 	= 5 ;

	sockaddr_in serverAddr ;

	memset(&serverAddr, 0x00, sizeof(sockaddr_in)) ;

	serverAddr.sin_family 		= AF_INET ;
	serverAddr.sin_addr.s_addr	= inet_addr(SERVER_IP) ;
	serverAddr.sin_port 		= htons(TCP_PORT) ;

	char szBuf[1024] ;

	int flagStatus = INITIALIZE_SOCKET ;

	int ret ;
	int fd ;
//	int fdMax = 0 ;
	int nread ;

	fd_set checkFds ;

	while(1)
	{
		if(m_flagDestroy)
			break ;

		switch(flagStatus)
		{
		case INITIALIZE_SOCKET :
			initSocket() ;

			flagStatus = CLIENT_IS_TRYING_CONNECT ;
			break ;
		case CLIENT_IS_TRYING_CONNECT :
			printf("CLIENT_IS_TRYING_CONNECT\n") ;
			ret = connect(m_clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) ;

			printf("RET : %d\n", ret) ;
			printf("m_clientSocket : %d\n", m_clientSocket) ;

			if(ret >= 0)
			{
//				if(m_clientSocket + 1 > fdMax)
//					fdMax = m_clientSocket + 1 ;
				setMaxFD(m_clientSocket) ;
				flagStatus = CLIENT_IS_WAITING ;
			}
			else
			{
				printf("ERROR : connect is failed errno : %d\n", errno) ;
				usleep(300000) ;
			}
			break ;
		case CLIENT_IS_WAITING :
			printf("CLIENT_IS_WAITING\n") ;
			checkFds = m_fds ;
			select(m_clientSocket + 1, &checkFds, 0, 0, NULL) ;

			for(fd = 0; fd <= m_fdMax ; fd++)
			{
				if(!FD_ISSET(fd, &checkFds))
					continue ;

				if(FD_ISSET(m_pipe[0], &checkFds))
					break ;

				if(fd == m_clientSocket)
				{
					ret = ioctl(fd, FIONREAD, &nread) ;
					printf("FIONREAD size : %d\n", nread) ;

					printf("fd == m_clientSocket, nread : %d\n", nread) ;
					
					if(nread == 0)
						flagStatus = SERVER_CLOSES_CONNECTION  ;
					else
						flagStatus = CLIENT_RECEIVED_DATA ;
					break ;
				}
			}

			break ;
		case CLIENT_RECEIVED_DATA :
			printf("CLIENT_RECEIVED_DATA\n") ;
			nread = recv(m_clientSocket, szBuf, sizeof(szBuf), 0) ;
			printf("recv buf ; %s\n", szBuf) ;
			flagStatus = CLIENT_IS_WAITING ;
			break ;
		case SERVER_CLOSES_CONNECTION :
			printf("SERVER_CLOSES_CONNECTION\n") ;
			FD_CLR(m_clientSocket, &m_fds) ;
			close(m_clientSocket) ;
			m_clientSocket = -1 ;

			flagStatus = INITIALIZE_SOCKET ;
			break ;
		}
		usleep(1000) ;
	}

	return 1 ;
}

