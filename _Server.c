#pragma pack(2)

typedef struct
{
    char signature[2];
    unsigned int fileSize;
    unsigned int reserved;
    unsigned int offset;
} BmpHeader;

typedef struct
{
    unsigned int headerSize;
    unsigned int width;
    unsigned int height;
    unsigned short planeCount;
    unsigned short bitDepth;
    unsigned int compression;
    unsigned int compressedImageSize;
    unsigned int horizontalResolution;
    unsigned int verticalResolution;
    unsigned int numColors;
    unsigned int importantColors;

} BmpImageInfo;

typedef struct
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    //unsigned char reserved; Removed for convenience in fread; info.bitDepth/8 doesn't seem to work for some reason
} Rgb;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "transfer.h"

#include <stdlib.h>
#include <stdio.h>

struct threeNum
{
    int n1, n2, n3;
};

void pixel_mat(char *img);
void writefile(int sockfd, FILE *fp);
ssize_t total = 0;
int main(int argc, char *argv[])
{
    int sockfd = socket(2, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Can't allocate sockfd");
        exit(1);
    }

    struct sockaddr_in clientaddr, serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = 2;
    serveraddr.sin_addr.s_addr = inet_addr("192.168.1.149"); // htonl(INADDR_ANY);
    serveraddr.sin_port = htons(8877);                       //htons(SERVERPORT)

    if (bind(sockfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("Bind Error");
        exit(1);
    }

    if (listen(sockfd, 7788) == -1) //listen(sockfd, LINSTENPORT)
    {
        perror("Listen Error");
        exit(1);
    }

    socklen_t addrlen = sizeof(clientaddr);

    printf("Server : Running\n");
    int connfd;
    int running = 1;
    while (running)
    {
        if ((connfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
        {
            perror("accept");
            exit(1);
        }
        else
        {
            puts("New conection established");
        }
        /* if (connfd == -1)
        {
            perror("Connect Error");
            exit(1);
        }
        close(sockfd); */

        char filename[4096] = {0};                 //[BUFFSIZE]
        if (recv(connfd, filename, 4096, 0) == -1) //recv(connfd, filename, BUFFSIZE, 0)
        {
            perror("Can't receive filename");
            exit(1);
        }

        FILE *fp = fopen(filename, "wb");
        if (fp == NULL)
        {
            perror("Can't open file");
            exit(1);
        }

        char addr[16];                                                                                      //char addr[INET_ADDRSTRLEN];
        printf("Start receive file: %s from %s\n", filename, inet_ntop(2, &clientaddr.sin_addr, addr, 16)); //inet_ntop(AF_INET,
        writefile(connfd, fp);
        /* printf("Receive Success, NumBytes = %ld\n", total); */

        fclose(fp);
        close(connfd);

        int n;
        struct threeNum num;
        FILE *fptr;

        if ((fptr = fopen(filename, "rb")) == NULL)
        {
            printf("Error! opening file");

            // Program exits if the file pointer returns NULL.
            exit(1);
        }

        for (n = 1; n < 5; ++n)
        {
            fread(&num, sizeof(struct threeNum), 1, fptr);
        }
        fclose(fptr);
        pixel_mat(filename);
        /* char image_print[BUFFSIZE] = "eog ";
        strcat(image_print, filename);
        system(image_print); // Show window with image */
    }

    return 0;
}

void writefile(int sockfd, FILE *fp)
{
    ssize_t n;
    char buff[4096] = {0}; //buff[MAX_LINE]
    while ((n = recv(sockfd, buff, 4096, 0)) > 0)
    {
        total += n;
        if (n == -1)
        {
            perror("Receive File Error");
            exit(1);
        }

        if (fwrite(buff, sizeof(char), n, fp) != n)
        {
            perror("Write File Error");
            exit(1);
        }
        memset(buff, 0, 4096);
    }
}

void pixel_mat(char *img)
{
    FILE *inFile;
    BmpHeader header;
    BmpImageInfo info;
    Rgb *palette;
    Rgb *p;

    int i = 0;

    int R = 0;
    int G = 0;
    int B = 0;

    /* printf( "Opening file %s for reading.\n", img ); */

    inFile = fopen(img, "rb");
    if (!inFile)
    {
        printf("Error opening file %s.\n", img);
        /* return -1; */
    }

    if (fread(&header, 1, sizeof(BmpHeader), inFile) != sizeof(BmpHeader))
    {
        puts("Error reading bmp header.\n");
        /* return -1; */
    }

    if (fread(&info, 1, sizeof(BmpImageInfo), inFile) != sizeof(BmpImageInfo))
    {
        puts("Error reading image info.\n");
        /* return -1; */
    }

    if (info.numColors > 0)
    {
        puts("Reading palette.\n");
        palette = (Rgb *)malloc(sizeof(Rgb) * info.numColors);
        if (fread(palette, sizeof(Rgb), info.numColors, inFile) != (info.numColors * sizeof(Rgb)))
        {
            puts("Error reading palette.\n");
            /* return -1; */ // error
        }
    }

    Rgb *pixel = (Rgb *)malloc(sizeof(Rgb));
    int read, j;
    int total_pixcel = info.height * info.width;

    int p_p = 0;

    for (j = 0; j < info.height; j++)
    {
        read = 0;
        for (i = 0; i < info.width; i++)
        {
            if (fread(pixel, 1, sizeof(Rgb), inFile) != sizeof(Rgb))
            {
                puts("Error reading pixel!\n");
                exit(1);

                return -1;
            }
            read += sizeof(Rgb);

            if (pixel->red > pixel->green && pixel->red > pixel->blue)
            {
                R = R + 1;
            }
            else if (pixel->green > pixel->red && pixel->green > pixel->blue)
            {
                G = G + 1;
            }
            else
            {
                B = B + 1;
            }

        }
        if (read % 4 != 0)
        {
            read = 4 - (read % 4);
            fread(pixel, read, 1, inFile);
        }
    }

    if (R >= G && R >= B)
    {
        puts("--- > RED.\n");
    }
    else if (G >= R && G >= B)
    {
        puts("--- > GREEN.\n");
    }
    else
    {
        puts("--- > BLUE.\n");
    }

    fclose(inFile);
    /*     fclose(outFile);
    fclose (fp_output); */

    /* printf( "BMP-Info:\n" );
    printf( "Width x Height: %i x %i\n", info.width, info.height ); */
    /* printf( "Depth: %i\n", (int)info.bitDepth ); */
}