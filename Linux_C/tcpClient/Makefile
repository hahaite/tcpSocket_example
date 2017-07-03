CFLAGS += -W -Wall
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wno-unused-variable

main : main.c tcpClient.c
	gcc -o $@ $^ -lpthread $(CFLAGS)

clean :
	rm -f main *.o
