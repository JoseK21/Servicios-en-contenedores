FROM ubuntu:20.04

# WORKDIR /tarea1

RUN mkdir tarea1

# COPY . /tarea1

ADD a.out /tarea1

EXPOSE 8080 
#Puerto

VOLUME /tarea1/

CMD [ "/tarea1/a.out" ]