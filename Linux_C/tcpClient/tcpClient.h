#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_

#include <pthread.h>
#include <sys/types.h>


int TCPClient_init() ;
int TCPClient_destroy() ;

int TCPClient_sendData() ;



#endif
