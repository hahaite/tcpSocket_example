#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_

#include <pthread.h>
#include <sys/types.h>

class CTCPClient
{
public :
	static CTCPClient* getInstance() ;

	int init() ;
	int destroy() ;

	int waitingRecv() ;

	int sendData() ;
private :
	int initSocket() ;
	int setMaxFD(int newFD) ;

public :


private :
	int m_flagDestroy ;

	int m_clientSocket ; // my socket.

	pthread_t 	m_threadRecv ;

	pthread_mutex_t	m_mutex ;

	int m_pipe[2] ;
	int m_fdMax ;

	fd_set 		m_fds ;
};


#endif
