# Tarea 1
Servicios en contenedores

## Compilacion
    make
    
### Uso del sistema

    ./server
    ./client

### Para verificar el color predominante 
    https://www.imgonline.com.ua/eng/get-dominant-colors-result.php

### Generar imagen 
    docker build -t image1 .
    # Esto en el folder del proyecto

### Remover container
    docker rm name_container

### Remover imagen
    docker rmi name_image

### Detener imagen
    docker stop name_image

### Iniciar imagen
    docker start name_image

### Subir imagen a Docker Hub
    docker build -t josek21/image1 .
    docker login
    docker push josek21/image1

### Bajar imagen
    docker pull josek21/image1


ubuntu:20.04