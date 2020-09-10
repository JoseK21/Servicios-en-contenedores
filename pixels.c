#include <stdio.h>
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


int main( int argc, char **argv ) {

    FILE *inFile;
    BmpHeader header;
    BmpImageInfo info;
    Rgb *palette;
    int i = 0;

    int R, G, B = 0;

    printf( "Opening file %s for reading.\n", argv[1] );

    inFile = fopen( argv[1], "rb" );
    if( !inFile ) {
        printf( "Error opening file %s.\n", argv[1] );
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

    printf( "Opening file %s for writing.\n", argv[2] );
    FILE *outFile = fopen( argv[2], "wb" );
    if( !outFile ) {
        printf( "Error opening outputfile.\n" );
        return -1;
    }
    Rgb *pixel = (Rgb*) malloc( sizeof(Rgb) );
    int read, j;
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
            /* printf( "Pixel %d: %3d %3d %3d\n", i+1, pixel->red, pixel->green, pixel->blue ); */
        }
        if( read % 4 != 0 ) {
            read = 4 - (read%4);
            printf( "Padding: %d bytes\n", read );
            fread( pixel, read, 1, inFile );
        }
    }

    printf( "Done.\n" );
    printf( "COLORES: %3d %3d %3d\n", R, G, B );

    if ( R >= G && R >= B)
    {
        printf( "--- > RED.\n" );
    }
    else if ( G >= R && G >= B)
    {
        printf( "--- > GREEN.\n" );
    }
    else
    {
        printf( "--- > BLUE.\n" );
    }
    

    fclose(inFile);
    fclose(outFile);

    printf( "\nBMP-Info:\n" );
    printf( "Width x Height: %i x %i\n", info.width, info.height );
    printf( "Depth: %i\n", (int)info.bitDepth );

    return 0;

}