CFLAGS += -W -Wall
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wno-unused-variable

main : main.c tcpServer.c
	gcc -o $@ $^ -lpthread $(CFLAGS)

clean :
	rm -f main *.o
