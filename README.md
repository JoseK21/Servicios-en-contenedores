# Tarea 1
Servicios en contenedores

## Compilacion
    make
    
### Uso del sistema localmente

    ./server
    ./client 127.0.0.1

### Para verificar el color predominante 
    https://www.imgonline.com.ua/eng/get-dominant-colors-result.php

### Remover container
    docker container rm <ID> -f

### Remover imagen
    docker rmi <name_image>

### Detener imagen
    docker stop <name_image>

### Iniciar imagen
    docker start <name_image>

### Subir imagen a Docker Hub
    docker build -t josek21/<name_image> .
    docker login
    docker push josek21/<name_image>

### Bajar imagen
    docker pull josek21/image1

### Ingresar a la
    docker exec -it <name_container> bash

### Mostrar procesos
    top

### Generar imagen 
    docker build -t <image_name> .
    # Esto en el folder del proyecto
    # A partir del Docketfile

### Ejecutar contenedor
    docker run --publish 8000:8080 -v /home/jose/Desktop/TAREA1:/tarea1/ --detach --name <name_container> t4

### Mostrar contenedores creados
    docker container ls -a

### Pasos para limpieza
    1. eliminar los contenedores
    2. eliminar las imagenes

### Estructura de configuracion.config
    La lista de ips se separan por lineas en blanco, cada item de una lista de ips se divide unicamente por salto de linea.
    Es necesario dejar mas de 2 saltos de lineas al final del file, ya que es necesario para determinar cuando la ip no existe en lo absoluto