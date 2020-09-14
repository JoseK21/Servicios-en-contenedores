all : send_file receive_file pixel_algorithm
.PHONY : all
send_file : send_file.c transfer.h
	gcc -Wall -O2 send_file.c -o send_file
receive_file : receive_file.c transfer.h
	gcc -Wall -O2 receive_file.c -o receive_file
pixel_algorithm : pixels.c
	gcc pixels.c
clean :
	rm send_file receive_file