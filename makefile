all : _Client _Server
.PHONY : all
_Client : _Client.c
	gcc _Client.c -o client
_Server : _Server.c
	gcc _Server.c -o a.out
clean :
	rm client a.out
