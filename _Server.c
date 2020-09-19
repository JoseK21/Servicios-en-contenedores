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
} Rgb;

struct threeNum
{
    int n1, n2, n3;
};

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>

void pixel_mat(char *img);
void writefile(int sockfd, FILE *fp);
int createFiles();
int read_ips(char *myIP);
int move_file(char *sourcePath, int folder);
void printc(char *msg, int color);

ssize_t total = 0;
int main(int argc, char *argv[])
{
    createFiles();

    int sockfd = socket(2, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Can't allocate sockfd");
        exit(1);
    }

    struct sockaddr_in clientaddr, serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = 2;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(8080);

    if (bind(sockfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("Bind Error");
        exit(1);
    }

    if (listen(sockfd, 7788) == -1)
    {
        perror("Listen Error");
        exit(1);
    }

    socklen_t addrlen = sizeof(clientaddr);

    printf("\nServer: ");
    printc("Running\n", 3);

    int connfd;
    char buf[32];
    int running = 1;
    int newsockfd;
    while (running)
    {
        connfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen);
        char *ippp = inet_ntoa(clientaddr.sin_addr);
        int resultIP = read_ips(ippp);
        printf("Analisis de file; %d\n", resultIP);

        if (connfd < 0)
        {
            perror("accept");
            exit(1);
        }

        char filename[4096] = {0};
        if (recv(connfd, filename, 4096, 0) == -1)
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

        char addr[16];
        printf("\nFile name: %s \nFrom %s\n", filename, inet_ntop(2, &clientaddr.sin_addr, addr, 16));
        writefile(connfd, fp);

        fclose(fp);
        close(connfd);

        int n;
        struct threeNum num;
        FILE *fptr;

        if ((fptr = fopen(filename, "rb")) == NULL)
        {
            printf("Error! opening file");
            exit(1);
        }

        for (n = 1; n < 5; ++n)
        {
            fread(&num, sizeof(struct threeNum), 1, fptr);
        }
        fclose(fptr);
        pixel_mat(filename);
    }
    close(sockfd);
    return 0;
}

void writefile(int sockfd, FILE *fp)
{
    ssize_t n;
    char buff[4096] = {0};
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

    int i = 0;
    int R = 0;
    int G = 0;
    int B = 0;

    inFile = fopen(img, "rb");
    if (!inFile)
    {
        printf("Error opening file %s.\n", img);
        exit(1);
    }

    if (fread(&header, 1, sizeof(BmpHeader), inFile) != sizeof(BmpHeader))
    {
        puts("Error reading bmp header.\n");
        exit(1);
    }

    if (fread(&info, 1, sizeof(BmpImageInfo), inFile) != sizeof(BmpImageInfo))
    {
        puts("Error reading image info.\n");
        exit(1);
    }

    if (info.numColors > 0)
    {
        puts("Reading palette.\n");
        palette = (Rgb *)malloc(sizeof(Rgb) * info.numColors);
        if (fread(palette, sizeof(Rgb), info.numColors, inFile) != (info.numColors * sizeof(Rgb)))
        {
            puts("Error reading palette.\n");
            exit(1);
        }
    }

    Rgb *pixel = (Rgb *)malloc(sizeof(Rgb));
    int read, j;

    for (j = 0; j < info.height; j++)
    {
        read = 0;
        for (i = 0; i < info.width; i++)
        {
            if (fread(pixel, 1, sizeof(Rgb), inFile) != sizeof(Rgb))
            {
                puts("Error reading pixel!\n");
                exit(1);
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

    printf("Image > ");
    if (R >= G && R >= B)
    {
        printc("RED.\n", 1);
        move_file(img, 1);
    }
    else if (G >= R && G >= B)
    {
        printc("GREEN.\n", 3);
        move_file(img, 2);
    }
    else
    {
        printc("BLUE.\n", 2);
        move_file(img, 3);
    }

    fclose(inFile);
}

int createFiles()
{
    char root_path[] = "server-storage";
    FILE *fp = fopen("server-storage/R", "r");
    if (!fp)
    {
        system("mkdir server-storage");
        system("mkdir -p server-storage/R");
        system("mkdir -p server-storage/G");
        system("mkdir -p server-storage/B");
        system("mkdir -p server-storage/'Not trusted'");
        return 1;
    }
    return 0;
}

int move_file(char *sourcePath, int folder)
{
    char destPath[100] = "server-storage/";

    if (folder == 1)
    {
        strcat(destPath, "R/");
        strcat(destPath, sourcePath);
    }
    else if (folder == 2)
    {
        strcat(destPath, "G/");
        strcat(destPath, sourcePath);
    }

    else if (folder == 3)
    {
        strcat(destPath, "B/");
        strcat(destPath, sourcePath);
    }

    else
    {
        strcat(destPath, "'Not trusted/'");
        strcat(destPath, sourcePath);
    }

    char mv_string1[100] = "";
    strcat(mv_string1, "mv ");
    strcat(mv_string1, sourcePath);
    strcat(mv_string1, " ");
    strcat(mv_string1, destPath);
    system(mv_string1);

    return 0;
}

int read_ips(char *myIP)
{
    int clasIP = 0;
    static const char filename[] = "configuracion.config";
    FILE *file = fopen(filename, "r");
    if (file != NULL)
    {
        char line[128];                                /* or other suitable maximum line size */
        while (fgets(line, sizeof line, file) != NULL) /* read a line */
        {

            int len = strlen(line);
            if (line[len - 1] == '\n')
            {
                line[len - 1] = 0;
            }

            int xx = strcmp(line, "");
            if (xx == 0)
            {
                clasIP++;
            }

            int x = strcmp(line, myIP);
            printf("Comparacion: %d\n", x); // prints 1

            // SEPARA LAS IPS Y ENVIAR UN NUMERO DEPENDIENDO EL WHILE

            if (x == 0)
            {
                printf("Encontrada en orden : %d \n", clasIP);
                break;
            }
        }
        fclose(file);
    }
    else
    {
        perror(filename); /* why didn't the file open? */
    }
    return clasIP;
}

void printc(char *msg, int color)
{
    if (color == 1)
    {
        printf("\033[1;31m");
        printf("%s", msg);
        printf("\033[0m");
    }
    else if (color == 2)
    {
        printf("\033[1;34m");
        printf("%s", msg);
        printf("\033[0m");
    }
    else if (color == 3)
    {
        printf("\033[1;32m");
        printf("%s", msg);
        printf("\033[0m");
    }
    else if (color == 4)
    {
        printf("\033[1;33m");
        printf("%s", msg);
        printf("\033[0m");
    }
    else if (color == 5)
    {
        printf("\033[1;35m");
        printf("%s", msg);
        printf("\033[0m");
    }
    else if (color == 6)
    {
        printf("\033[1;36m");
        printf("%s", msg);
        printf("\033[0m");
    }
}

// configuracion.config
