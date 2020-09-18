#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <png.h>

const int  BUFFER_SIZE = 1023;
const char INCOMPLETE_MSG[] = "INCOMPLETE";
const char COMPLETE_MSG[] = "COMPLETE";
const char END_MSG[] = "END";
const char PROCESS_COMPLETE_MSG[] = "PROCESS_COMPLETE";
const char DATA_PROCESSING_PATH[] = "psot1-dprocessing/";
const char ERROR[] = "*$ERROR";

void listenClient(int serverSocket, char* dfolderpath);
char* receiveFile(int socket); 
char* long2str(int number);
char* concat(const char *s1, const char *s2);
int createDirectory(char* filepath);
char* createDataFolders();
long maxOfTwo(long a, long b);
int getPredominantColor(long sumR, long sumG, long sumB);
int processImage(char* filepath);
int copyFile(char* filepath, char* dst_path);
void saveImage(char* filename, char* dst_path, int color);
int classifyImage(char* dfolderpath, char* filename);

int main() {
    // Creacion de directorios
    char* dfolderpath = createDataFolders(); // Guardar ruta del folder de almacenamiento
    printf("Ruta: %s\n\n", dfolderpath);

    // Creacion del descriptor del socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    // Configuracion de direccion y puerto del servidor
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080); // Puerto
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Direccion IP
    
    // Se asigna el puerto al socket
    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    // Escucha de conexiones entrantes
    listen(serverSocket, 5);

    while (1) {
        listenClient(serverSocket, dfolderpath);
    }

    // Cerrar la conexion
    close(serverSocket);
    free(dfolderpath);
    return 0;
}

void listenClient(int serverSocket, char* dfolderpath) {
    // Estructura para obtener la informacion del cliente
    struct sockaddr_in clientAddr;
    unsigned int sin_size = sizeof(clientAddr);

    // Se espera por una conexion con un cliente
    int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sin_size);
    char *ipClient = inet_ntoa(clientAddr.sin_addr);
    printf("Cliente %s conectado!\n", ipClient);

    unsigned char* buffer = (char*) malloc(sizeof(unsigned char)*BUFFER_SIZE);
    while (1) {
        memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);
        int m = recv(clientSocket, buffer, BUFFER_SIZE, 0); // Se espera por el mensaje de inicio

        if (m == 0) { // Se perdio la conexion con el cliente
            break;
        }

        // Recepcion del archivo
        char* filename = receiveFile(clientSocket);

        // Clasificacion del archivo
        if (strcmp(filename, ERROR) != 0) {
            classifyImage(dfolderpath, filename);
        }

        // Mensaje de finalizacion de proceso
        send(clientSocket, PROCESS_COMPLETE_MSG, BUFFER_SIZE, 0);
    }
    printf("Conexion perdida! Esperando nueva conexion...\n\n");
}

/**
 * Funcion para recibir un archivo proveniente de un socket clietne
 * socket: descriptor del socket cliente
 * return: ruta del archivo recibido
*/ 
char* receiveFile(int socket) {
    // Mensaje del cliente con el nombre y dimension del archivo
    unsigned char clientMessage[BUFFER_SIZE];
    int r = recv(socket, &clientMessage, BUFFER_SIZE, 0);

    if(strcmp(clientMessage, ERROR) == 0) { // Ocurrio un error con el cliente
        return (char*)ERROR;
    }

    char *filename = strtok(clientMessage, "*");
    char *fileSize = strtok(NULL, "*");

    char* name = malloc(strlen(filename) + 1);
    strcpy(name, filename);
    filename = name;

    // Apertura del archivo de escritura
    FILE *write_ptr;
    char* filepath = concat(DATA_PROCESSING_PATH, filename);
    write_ptr = fopen(filepath,"wb");

    // Calculo aproximado de iteraciones necesarias
    int iter = (atoi(fileSize)/BUFFER_SIZE);
    int iter_max = iter + 5; // Para asegurar la recepcion completa

    // Recepcion del archivo
    for (int i = 0; i < iter_max; i++) {
        // Creacion del buffer para leer el mensaje enviado por el socket cliente
        unsigned char* buffer = (char*) malloc(sizeof(unsigned char)*BUFFER_SIZE);
        memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);

        // Lectura del mensaje entrante
        int readBytes = recv(socket, buffer, BUFFER_SIZE, 0);

        if (readBytes == 0) { // Conexion perdida
            fclose(write_ptr);
            free(buffer);
            free(name);
            remove(filepath);
            return (char*)ERROR;
        }

        // Se verifica si ha finalizado el envio del archivo
        if (strcmp(END_MSG, buffer) == 0) {
            break;
        }
        
        // Se verifica si se recibio el mensaje correctamente (bytes recibidos = BUFFER_SIZE)
        if(readBytes != BUFFER_SIZE && i < iter) {
            int completed = 0;
            while (!completed) {
                // Envio del mensaje indicando una recepcion incompleta
                int s = send(socket, INCOMPLETE_MSG, BUFFER_SIZE, 0);

                if (s == 0) { // Conexion perdida
                    fclose(write_ptr);
                    free(buffer);
                    free(name);
                    remove(filepath);
                    return (char*)ERROR;
                }

                // Se restablece la direccion de memoria del buffer
                memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);

                // Se recibe de nuevo el mensaje
                readBytes = recv(socket, buffer, BUFFER_SIZE, 0);

                // Se verifica si el nuevo mensaje se recibio correctamente
                if (readBytes == BUFFER_SIZE) {
                    completed = 1;
                    fwrite(buffer,readBytes,1,write_ptr);   
                    send(socket, COMPLETE_MSG, BUFFER_SIZE, 0);
                }
            }
        } else {
            // Se almacena la informacion recibida
            fwrite(buffer,readBytes,1,write_ptr);
            send(socket, COMPLETE_MSG, BUFFER_SIZE, 0);
        }

        // Limpieza de memoria
        free(buffer); 
    }   
    fclose(write_ptr);
    return filename;
}

/**
 * Funcion para obtener un string a partir de un numero
 * number: numero que se desea convertir a string
 * return: string que representa al numero
*/
char* long2str(int number) {
    int n = snprintf(NULL, 0, "%d", number);
    char* buf = malloc(n+1);
    snprintf(buf, n+1, "%d", number);
    return buf;
}

/**
 * Funcion para concatenar un string s2 al final de un string s1
 * s1: string que se desea concatenar
 * s2: string que se desea concatenar
 * return: string con la concatenacion de s1 y s2
*/ 
char* concat(const char *s1, const char *s2) {
    const size_t len1 = strlen(s1); // Largo de s1
    const size_t len2 = strlen(s2); // Largo de s2
    char *result = malloc(len1 + len2 + 1); // +1 para el null-terminator
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);
    return result;
}

/**
 * Funcion para crear un directorio
 * filepath: ruta con la direccion y nombre de la carpeta
 * return: 0 en caso exitoso, -1 si hubo un error
*/ 
int createDirectory(char* filepath) {
    return mkdir(filepath, 0777);
}

/**
 * Funcion para crear los directorios de almacenamiento de imagenes procesadas
 * return: path a la carpeta correspondiente al contenedor
*/  
char* createDataFolders() {
    // Directorio para almacenamiento de imagenes por procesar
    createDirectory((char*) DATA_PROCESSING_PATH);

    // Directorios para almacenamiento de las imagenes
    createDirectory("psot1-dstorage");
    char *sCounter = NULL;
    char *name = NULL;
    int created = -1; 
    int counter = 0;

    // Se determina el numero de contenedor correspondiente
    while (created != 0){
        counter++;
        sCounter = long2str(counter);
        name = concat("psot1-dstorage/container", sCounter);
        created = createDirectory(name);
    }
    // Creacion de los nombres de las carpetas
    char *redFolder = concat(name,"/R");
    char *greenFolder = concat(name,"/G");
    char *blueFolder = concat(name,"/B");
    char *notTrustedFolder = concat(name,"/Not trusted");

    // Creacion de directorios
    createDirectory(redFolder);
    createDirectory(greenFolder);
    createDirectory(blueFolder);
    createDirectory(notTrustedFolder);

    // Se guarda en memoria la ruta de almacenamiento
    char* path = malloc(strlen(name) + 1);
    strcpy(path, name);
    return path;
}

/**
 * Funcion para calcular el canal dominante de una imagen
 * filepath: ruta de la imagen a procesar
 * return: numero que indica el canal dominante
 *         1: canal rojo, 2: canal verde, 3: canal azul
*/ 
int processImage(char* filename) {
    char* filepath = concat(DATA_PROCESSING_PATH, filename);
    FILE *pFile = fopen(filepath, "rb");
    if(!pFile) {
        printf("Error al leer el archivo %s\n", filepath);
        return -1;
    }
    // Se leen los primeros 8 bits del archivo para
    // verificar que sea una imagen en formato png
    int len = 8; // Largo del buffer
    char header[len]; // Buffer
    fread(header, 1, len, pFile); // Lectura de los primeros 8 bits
    int is_png = !png_sig_cmp(header, 0, len); 
    if (!is_png) {
        printf("Archivo %s no es una imagen en formato png\n", filename);
        fclose(pFile);
        remove(filepath); // Se elimina el archivo de la carpeta de procesamineto
        return -1;
    }

    // Creacion de la estructura de lectura
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(setjmp(png_jmpbuf(png_ptr))) {
        printf("Error al obtener la informacion del archivo %s\n", filepath);
        fclose(pFile);
        return -1;
    }

    // Lectura de la informacion de la imagen
    png_init_io(png_ptr, pFile);
    png_set_sig_bytes(png_ptr, len); // Se indica que se han leido 8 bits
    png_read_info(png_ptr, info_ptr);
    int width = png_get_image_width(png_ptr, info_ptr);   // Ancho
    int height = png_get_image_height(png_ptr, info_ptr); // Largo
    int channels = png_get_channels(png_ptr, info_ptr);   // Canales
    int number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    // Lectura de los datos
    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("Error durante la lectura de los pixeles\n");
        fclose(pFile);
        return -1;
    }

    // Memoria para almacenar los pixeles de la imagen
    png_bytep *row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (int i = 0; i < height; i++) {
        row_pointers[i] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
    }

    // Lectura de los pixeles
    png_read_image(png_ptr, row_pointers);
    long sumR = 0, sumG = 0, sumB = 0; // Sumatoria de cada canal
    for (int y = 0; y < height; y++) {
        png_byte* row = row_pointers[y];
        for (int x = 0; x < width; x++) {
            png_byte* pixel = &(row[x*3]);
            // Suma del pixel a la sumatoria del canal correspondiente
            sumR += pixel[0];
            sumG += pixel[1];
            sumB += pixel[2];
        }
    }

    // Calculo del color predominante
    int predominant = getPredominantColor(sumR, sumG, sumB);

    // Limpieza de memoria
    fclose(pFile);
    free(row_pointers);

    return predominant;
}

/**
 * Funcion que calcula el maximo entre dos numeros
 * a: primer numero de la comparacion
 * b: segundo numero de la comparacion
 * return: el numero mas grande entre a y b
*/ 
long maxOfTwo(long a, long b) {
    return (a > b) ? a : b; 
}

/**
 * Funcion que calcula indica el canal predominante de una imagen
 * sumR: sumatoria de los valores del canal rojo
 * sumG: sumatoria de los valores del canal verde
 * sumB: sumatoria de los valores del canal azul
 * return: 1 canal rojo, 2 canal verde, 3 canal azul
*/
int getPredominantColor(long sumR, long sumG, long sumB) {    
    // Se calcula el numero maximo
    long biggest = maxOfTwo(maxOfTwo(sumR,sumG),sumB);
    // Se identifica cual canal tiene la sumatoria mas grande
    if (sumR == biggest) {
        return 1;
    } else if (sumG == biggest) {
        return 2;
    } else {
        return 3;
    }
}

/**
 * Funcion para copiar un archivo en una ruta especifica, elimina el original
 * filepath: ruta del archivo que se desea copiar
 * dst_path: ruta destino para el nuevo archivo
 * return: 0 si todo salio bien, -1 si hubo un error
*/ 
int copyFile(char* filepath, char* dst_path) {
    // Lectura del archivo original
    FILE *pFile = fopen(filepath, "rb");
    if(!pFile) {
        printf("Copia: Error al abrir el archivo original\n");
        return -1;
    }

    // Apertura del nuevo archivo en la direccion deseada
    FILE *write_ptr = write_ptr = fopen(dst_path,"wb");
    if(!write_ptr) {
        printf("Copia: Error al abrir el archivo destino\n");
        return -1;
    }

    //Creacion del buffer
    unsigned char* buffer = (char*) malloc(sizeof(unsigned char)*BUFFER_SIZE);

    // Copia del archivo
    while (1) {
        // Limpieza del buffer
        memset(buffer, 0, sizeof(unsigned char)*BUFFER_SIZE);

        // Lectura del archivo
        int readBytes = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, pFile);
        
        // Escritura del archivo
        fwrite(buffer, readBytes, 1, write_ptr); 

        // Condicion de parada
        if (readBytes != BUFFER_SIZE) {
            break;
        }
    }
    // Limpieza de memoria
    fclose(pFile);
    fclose(write_ptr);
    free(buffer);
    remove(filepath);
    
    return 0;
}

/**
 * Funcion auxiliar para almacenar la imagen en la carpeta que le corresponde
 * filename: nombre de la imagen recibida
 * dst_path: ruta a la carpeta del contenedor
 * color: 1 canal rojo, 2 canal verde, 3 canal azul, cualquier otro imagen no confiable
*/ 
void saveImage(char* filename, char* dst_path, int color) {
    char* filepath = concat(DATA_PROCESSING_PATH, filename);
    char* dest = NULL;
    switch (color) {
    case 1: // Canal dominante rojo
        dest = concat(dst_path, "/R/");
        dest = concat(dest, filename);
        copyFile(filepath, dest);
        printf("Imagen %s procesada, color dominante: ROJO\n", filename);
        break;
    
    case 2: // Canal dominante verde
        dest = concat(dst_path, "/G/");
        dest = concat(dest, filename);
        copyFile(filepath, dest);
        printf("Imagen %s procesada, color dominante: VERDE\n", filename);
        break;
    
    case 3: // Canal dominante azul
        dest = concat(dst_path, "/B/");
        dest = concat(dest, filename);
        copyFile(filepath, dest);
        printf("Imagen %s procesada, color dominante: AZUL\n", filename);
        break;

    default: // Imagen de fuente no confiable
        dest = concat(dst_path, "/Not trusted/");
        dest = concat(dest, filename);
        copyFile(filepath, dest);
        printf("Imagen %s no procesada, fuente no confiable\n", filename);
        break;
    } 
}

/**
 * Funcion para almacenar la imagen recibida en la carpeta
 * correspondiente segun su canal dominante
 * dfolderpath: ruta a la carpeta raiz de almacenamiento de datos
 * filename: nombre de la imagen recibida
 * return: 0 sin errores, -1 ocurrio un error
*/ 
int classifyImage(char* dfolderpath, char* filename) {
    // Calculo del color dominante de la imagen
    int color = processImage(filename); 
    if(color == -1) {
        return -1;
    }
    // Almacenamiento de la imagen en el folder correspondiente
    saveImage(filename, dfolderpath, color);
    free(filename);
    return 0;
}