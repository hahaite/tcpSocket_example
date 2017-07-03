#include <stdio.h>
#include <unistd.h>

#include <termio.h>

#include "tcpClient.h"

int main()
{
	TCPClient_init() ;

	int flagBreak = 0 ;
	int szChar ;

	while(1)
	{

		szChar = getchar() ;

		switch(szChar)
		{
		case 's' :
			TCPClient_sendData() ;
			break ;
		case 'q' :
			printf("Break~!!\n") ;
			flagBreak = 1 ;
			break ;
		}

		if(flagBreak)
		{
			getchar() ;
			break ;
		}
	}

	usleep(1000) ;
	TCPClient_destroy() ;
	usleep(1000) ;

	return 1 ;
}


