#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "transfer.h"
#include "print_color.h"

void sendfile(FILE *fp, int sockfd);
ssize_t total = 0;
int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printc(">>>>", 4);
        puts("Usage: ./client <IPaddress>");

        exit(1);
    }

    while (1)
    {
        char path_img[100];
        printc("Enter your image path: ", 5);
        scanf("%s", path_img);

        if (strcmp(path_img, "fin") == 0)
        {
            break;
        }

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            perror("Can't allocate sockfd");
            exit(1);
        }

        struct sockaddr_in serveraddr;
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serveraddr.sin_port = htons(8888); 
        if (inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) < 0)
        {
            perror("IPaddress Convert Error");
            exit(1);
        }

        if (connect(sockfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
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
        printc("Done\n", 3);
        fclose(fp);
        close(sockfd);
    }
    printf("\nSee you...\n");
    return 0;
}

void sendfile(FILE *fp, int sockfd)
{
    int n;
    char sendline[MAX_LINE] = {0};
    while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0)
    {
        total += n;
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
