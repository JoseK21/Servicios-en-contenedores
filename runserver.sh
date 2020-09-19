

make clean
make
docker build -t is .
docker run --publish 8000:8080 -v /home/jose/Desktop/ImageStorage/:/server-storage/ --detach --name cs is

docker run --publish 8000:8080 -v /home/josek21/Desktop/ImageStorage/:/server-storage/ --detach --name cs josek21/tarea-1



docker exec -it cs bash
cd server-storage
ls -R


docker stop cs
docker container rm cs
docker rmi is
