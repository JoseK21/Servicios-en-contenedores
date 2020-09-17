all : _Client _Server
.PHONY : all
_Client : _Client.c transfer.h
	gcc -Wall -O2 _Client.c -o client
_Server : _Server.c transfer.h
	gcc -Wall -O2 _Server.c -o server
clean :
	rm client server
