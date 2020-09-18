FROM ubuntu:20.04

# Instalacion GCC y libpng
RUN apt-get update && \
    apt-get -y install gcc mono-mcs && \
    apt-get install -y libpng-dev && \
    rm -rf /var/lib/apt/lists/*

RUN cd home && mkdir psot1 && mkdir psot1-dstorage

VOLUME /psot1-dstorage/

ADD _Server.c /home/psot1

RUN cd /home/psot1 && gcc _Server.c -o a.out

EXPOSE 8080

CMD [ "/home/psot1/a.out" ]