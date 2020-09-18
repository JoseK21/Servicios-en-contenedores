all : _Client _Server
.PHONY : all
_Client : _Client.c
	gcc _Client.c -o client
_Server : _Server.c
	gcc _Server.c -o server
clean :
	rm client server
