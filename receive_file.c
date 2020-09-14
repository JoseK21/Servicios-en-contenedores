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


void pixel_mat(char *img );
void writefile(int sockfd, FILE *fp);
ssize_t total=0;
int main(int argc, char *argv[]) 
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) 
    {
        perror("Can't allocate sockfd");
        exit(1);
    }
    
    struct sockaddr_in clientaddr, serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);

    if (bind(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) == -1) 
    {
        perror("Bind Error");
        exit(1);
    }

    if (listen(sockfd, LINSTENPORT) == -1) 
    {
        perror("Listen Error");
        exit(1);
    }

    socklen_t addrlen = sizeof(clientaddr);
    int connfd = accept(sockfd, (struct sockaddr *) &clientaddr, &addrlen);
    if (connfd == -1) 
    {
        perror("Connect Error");
        exit(1);
    }
    close(sockfd); 

    char filename[BUFFSIZE] = {0}; 
    if (recv(connfd, filename, BUFFSIZE, 0) == -1) 
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
    
    char addr[INET_ADDRSTRLEN];
    printf("Start receive file: %s from %s\n", filename, inet_ntop(AF_INET, &clientaddr.sin_addr, addr, INET_ADDRSTRLEN));
    writefile(connfd, fp);
    printf("Receive Success, NumBytes = %ld\n", total);

    fclose(fp);
    close(connfd);
   

    int n;
    struct threeNum num;
    FILE *fptr;

    if ((fptr = fopen(filename,"rb")) == NULL){
        printf("Error! opening file");

        // Program exits if the file pointer returns NULL.
        exit(1);
    }

    for(n = 1; n < 5; ++n)
    {
        fread(&num, sizeof(struct threeNum), 1, fptr);
    }
    fclose(fptr); 
    pixel_mat(filename);
    char image_print[BUFFSIZE] = "eog ";
    strcat(image_print, filename);
    system(image_print); // Show window with image

    return 0;
}

void writefile(int sockfd, FILE *fp)
{
    ssize_t n;
    char buff[MAX_LINE] = {0};
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0) 
    {
	    total+=n;
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
        memset(buff, 0, MAX_LINE);
    }
}


void pixel_mat(char *img ) {

    FILE *inFile;
    BmpHeader header;
    BmpImageInfo info;
    Rgb *palette;
    int i = 0;

    int R, G, B = 0;

    printf( "Opening file %s for reading.\n", img );

    inFile = fopen( img, "rb" );
    if( !inFile ) {
        printf( "Error opening file %s.\n", img );
        return -1;
    }

    if( fread(&header, 1, sizeof(BmpHeader), inFile) != sizeof(BmpHeader) ) {
        printf( "Error reading bmp header.\n" );
        return -1;
    }

    if( fread(&info, 1, sizeof(BmpImageInfo), inFile) != sizeof(BmpImageInfo) ) {
        printf( "Error reading image info.\n" );
        return -1;
    }

    if( info.numColors > 0 ) {
        printf( "Reading palette.\n" );
        palette = (Rgb*)malloc(sizeof(Rgb) * info.numColors);
        if( fread(palette, sizeof(Rgb), info.numColors, inFile) != (info.numColors * sizeof(Rgb)) ) {
            printf( "Error reading palette.\n" );
            return -1; // error
        }
    }

/*     printf( "Opening file %s for writing.\n", argv[2] );
    FILE *outFile = fopen( argv[2], "wb" );
    if( !outFile ) {
        printf( "Error opening outputfile.\n" );
        return -1;
    } */
    Rgb *pixel = (Rgb*) malloc( sizeof(Rgb) );
    int read, j;
    int total_pixcel = info.height * info.width;
    /* FILE * fp_output; */
   /* open the file for writing*/
    /* fp_output = fopen ("output.txt","w"); */

    for( j=0; j<info.height; j++ ) {
        /* printf( "------ Row %d\n", j+1 ); */
        read = 0;
        for( i=0; i<info.width; i++ ) {
            if( fread(pixel, 1, sizeof(Rgb), inFile) != sizeof(Rgb) ) {
                printf( "Error reading pixel!\n" );
                return -1;
            }
            read += sizeof(Rgb);
            R = R + pixel->red;
            G = G + pixel->green;
            B = B + pixel->blue;
            printf( "Pixel %d: %d %3d %3d\n", i+1, pixel->red, pixel->green, pixel->blue );
            /* fprintf (fp_output, "Pixel %d: %d %3d %3d\n", i+1, pixel->red, pixel->green, pixel->blue); */
        }
        if( read % 4 != 0 ) {
            read = 4 - (read%4);
            /* printf( "Padding: %d bytes\n", read ); */
            fread( pixel, read, 1, inFile );
        }
    }

    printf( "Done.\n" );
    /* printf( "COLORES: %d %d %d\n", R, G, B ); */

    if ( R >= G && R >= B)
    {
        printf( "--- > RED.\n%d\n", R / total_pixcel);
    }
    else if ( G >= R && G >= B)
    {
        printf( "--- > GREEN.\n%d\n", G / total_pixcel);
    }
    else
    {
        printf( "--- > BLUE.\n%d\n", B / total_pixcel);
    }
    

    fclose(inFile);
/*     fclose(outFile);
    fclose (fp_output); */

    printf( "BMP-Info:\n" );
    printf( "Width x Height: %i x %i\n", info.width, info.height );
    /* printf( "Depth: %i\n", (int)info.bitDepth ); */

    return 0;

}