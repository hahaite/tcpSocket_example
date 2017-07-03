#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include <pthread.h>
#include <arpa/inet.h>


class CTCPServer
{
public :
	static CTCPServer* getInstance() ;


	int init() ;
	int destroy() ;

	int sendData() ;

	int waitingConnect() ;
private :
	int setMaxFD(int newFD) ;

public :

private :
	int m_flagDestroy ;

	pthread_t m_threadConnect ;

	int m_serverSocket ;
	int m_clientSocket ;

	int m_pipe[2] ;

	int m_fdMax ;

	fd_set 	m_fds ;
};

#endif

