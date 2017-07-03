#include <cstdio>
#include <unistd.h>

#include <termio.h>

#include "tcpClient.h"

int main()
{
	CTCPClient* client = CTCPClient::getInstance() ;
	client->init() ;

	int flagBreak = 0 ;
	int szChar ;

	while(1)
	{

		szChar = getchar() ;

		switch(szChar)
		{
		case 's' :
			client->sendData() ;
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
	client->destroy() ;
	usleep(1000) ;

	return 1 ;
}


