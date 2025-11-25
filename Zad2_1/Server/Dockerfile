FROM gcc:4.9
COPY . /Sockets
WORKDIR /Sockets/
RUN gcc -std=gnu99 -o server server.c
ENTRYPOINT ["./server"]