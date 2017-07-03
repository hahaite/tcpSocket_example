#include <cstdio>
#include <unistd.h>

#include <termio.h>

#include "tcpServer.h"


int main()
{
	CTCPServer* server = CTCPServer::getInstance() ;
	server->init() ;

	int flagBreak = 0 ;
	int szChar ;

	while(1)
	{
		szChar = getchar() ;	

		switch(szChar)
		{
		case 's' :
			server->sendData() ;
			break ;
		case 'q' :
			printf("BREAK\n") ;
			flagBreak = 1 ;
			break ;
		}

		if(flagBreak)
		{
			getchar() ;
			break ;
		}

	}

	printf("before server->destroy()\n") ;
	server->destroy() ;
	printf("after server->destroy()\n") ;

	return 1 ;
}


