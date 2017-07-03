CFLAGS += -W -Wall
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wno-unused-variable

main : main.cpp tcpClient.cpp
	g++ -o $@ $^ -lpthread $(CFLAGS)

clean :
	rm -f main *.o
