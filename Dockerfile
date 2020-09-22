FROM ubuntu:20.04

RUN apt-get update && \
    apt-get -y install gcc mono-mcs && \
    apt-get install -y libpng-dev && \
    rm -rf /var/lib/apt/lists/*

RUN cd home && mkdir server && mkdir server-storage && cd ..

ADD server /home/server

EXPOSE 8080

CMD [ "/home/server/server" ]
