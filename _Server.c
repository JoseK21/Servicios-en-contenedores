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
#include <dirent.h>

/* global variable declaration */
int id_container;

void pixel_mat(char *img);                   // algoritmo para determinar color de imagen
void writefile(int sockfd, FILE *fp);        // escritura de archivo (imagen) al contenedor
int createFiles();                           // creacion de folders
int read_ips(char *myIP);                    // lectura de ips (configuracion.config)
int move_file(char *sourcePath, int folder); // movimiento de archivos, del root a la carpeta respectiva (R-G-B)
void no_trusted(char *img);                  // movimiento de archivos, del root a la carpeta No Trusted, sin analisis de color principal
void printc(char *msg, int color);           // printf con colores

int main(int argc, char *argv[])
{
    createFiles();

    int sockfd = socket(2, SOCK_STREAM, 0); // Creacion de socket
    if (sockfd == -1)
    {
        perror("Can't allocate sockfd");
        exit(1);
    }

    /* Conficuracion de sockets address */
    struct sockaddr_in clientaddr, serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = 2;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(8080);

    // Verificacion de socket
    if (bind(sockfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("Bind Error");
        exit(1);
    }

    // verificion de puesto del socket
    if (listen(sockfd, 7788) == -1)
    {
        perror("Listen Error");
        exit(1);
    }

    socklen_t addrlen = sizeof(clientaddr);

    printf("\nServer: ");
    printc("Running\n", 3);

    int connfd;
    while (1) // Loop de server
    {
        connfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen); // verificacion de socket con cliente

        if (connfd < 0) // Socket no establecido
        {
            printf("No Connected");
        }
        else // Socket establecido
        {
            char *ippp = inet_ntoa(clientaddr.sin_addr);
            int resultIP = read_ips(ippp);
            printf("Analisis de file; %d\n", resultIP);

            if (resultIP <= 1)
            {
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

                if (resultIP == 0)
                {
                    pixel_mat(filename);
                }
                else if (resultIP == 1)
                {
                    no_trusted(filename);
                }
            }
            else
            {
                close(connfd); // cierre de socket - client
            }
        }
    }
    close(sockfd); // cierre de socket - serve
    return 0;
}

void writefile(int sockfd, FILE *fp)
{
    ssize_t n;
    char buff[4096] = {0};
    while ((n = recv(sockfd, buff, 4096, 0)) > 0)
    {
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

    /* Contadores de pixels por color */
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

    /* Determinacion de color -> folder */
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

void no_trusted(char *img)
{
    FILE *inFile;

    inFile = fopen(img, "rb");
    if (!inFile)
    {
        printf("Error opening file %s.\n", img);
        exit(1);
    }

    move_file(img, 4); // 4 : No Truted

    fclose(inFile);
}

int createFiles()
{
    char root_path[] = "server-storage";
    FILE *fp = fopen("server-storage", "r");
    if (!fp)
    {
        system("mkdir server-storage");
    }

    int container = 1;
    while (1)
    {
        char snum[5];
        char root[] = "server-storage/container-";

        sprintf(snum, "%d", container);
        strcat(root, snum);
        puts(root);

        DIR *dir = opendir(root);
        if (dir)
        {
            container = container + 1;
            closedir(dir);
        }
        else
        {
            id_container = container;
            char fl1[] = "mkdir -p ";
            strcat(fl1, root);
            system(fl1);

            size_t len = strlen(fl1);
            char flR[len];
            strcpy(flR, fl1);

            char flG[len];
            strcpy(flG, fl1);

            char flB[len];
            strcpy(flB, fl1);

            char flT[len];
            strcpy(flT, fl1);

            strcat(flR, "/R");
            system(flR);

            strcat(flG, "/G");
            system(flG);

            strcat(flB, "/B");
            system(flB);

            strcat(flT, "/'Not trusted'");
            system(flT);

            break;
        }
    }

    return 0;
}

int move_file(char *sourcePath, int folder)
{
    char destPath[100] = "server-storage/";
    strcat(destPath, "container-");

    char snum[5];

    sprintf(snum, "%d", id_container);

    strcat(destPath, snum);

    if (folder == 1)
    {
        strcat(destPath, "/R/");
        strcat(destPath, sourcePath);
    }
    else if (folder == 2)
    {
        strcat(destPath, "/G/");
        strcat(destPath, sourcePath);
    }

    else if (folder == 3)
    {
        strcat(destPath, "/B/");
        strcat(destPath, sourcePath);
    }

    else
    {
        strcat(destPath, "/'Not trusted/'");
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
    static const char filename[] = "server-storage/configuracion.config";
    FILE *file = fopen(filename, "r");
    if (file != NULL)
    {
        char line[128];
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
        return clasIP;
    }
    else
    {
        return 9;
    }
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
