

make clean
make
docker build -t is .
docker run --publish 8000:8080 -v /home/jose/Desktop/ImageStorage/:/server-storage/ --detach --name cs is


docker container rm cs
docker rmi is

docker exec -it cs bash