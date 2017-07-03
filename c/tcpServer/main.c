#include <stdio.h>
#include <unistd.h>

#include <termio.h>

#include "tcpServer.h"


int main()
{
	TCPServer_init() ;

	int flagBreak = 0 ;
	int szChar ;

	while(1)
	{
		szChar = getchar() ;	

		switch(szChar)
		{
		case 'q' :
			printf("BREAK\n") ;
			flagBreak = 1 ;
			break ;
		default :
			TCPServer_sendData(szChar) ;
			break ;

		}

		if(flagBreak)
		{
			getchar() ;
			break ;
		}

	}

	printf("before server->destroy()\n") ;
	TCPServer_destroy() ;
	printf("after server->destroy()\n") ;

	return 1 ;
}


