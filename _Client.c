#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "transfer.h"

void sendfile(FILE *fp, int sockfd);
ssize_t total=0;
int main(int argc, char* argv[])
{
    char my_ip[16];
    printf("Enter your my_ip\n");
    scanf("%s",my_ip);
    printf("IP %s\n",my_ip);

    char path_img[100];
    printf("Enter your path_img\n");
    scanf("%s",path_img);
    printf("Path Image %s\n",path_img);

/*     if (argc != 3) 
    {
        perror("usage:send_file filepath <IPaddress>");
        exit(1);
    } */

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("Can't allocate sockfd");
        exit(1);
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVERPORT);
    if (inet_pton(AF_INET, my_ip, &serveraddr.sin_addr) < 0)
    {
        perror("IPaddress Convert Error");
        exit(1);
    }

    if (connect(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("Connect Error");
        exit(1);
    }
    
    char *filename = basename(path_img); 
    if (filename == NULL)
    {
        perror("Can't get filename");
        exit(1);
    }
    
    char buff[BUFFSIZE] = {0};
    strncpy(buff, filename, strlen(filename));
    if (send(sockfd, buff, BUFFSIZE, 0) == -1)
    {
        perror("Can't send filename");
        exit(1);
    }
    
    FILE *fp = fopen(path_img, "rb");
    if (fp == NULL) 
    {
        perror("Can't open file");
        exit(1);
    }

    sendfile(fp, sockfd);
    //puts("Send Success");
    printf("Send Success, NumBytes = %ld\n", total);
    fclose(fp);
    close(sockfd);
    return 0;
}

void sendfile(FILE *fp, int sockfd) 
{
    int n; 
    char sendline[MAX_LINE] = {0}; 
    while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0) 
    {
	    total+=n;
        if (n != MAX_LINE && ferror(fp))
        {
            perror("Read File Error");
            exit(1);
        }
        
        if (send(sockfd, sendline, n, 0) == -1)
        {
            perror("Can't send file");
            exit(1);
        }
        memset(sendline, 0, MAX_LINE);
    }
}
