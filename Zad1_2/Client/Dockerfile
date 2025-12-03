FROM gcc:latest

RUN apt-get update && apt-get install -y iproute2

COPY . /Client
WORKDIR /Client

RUN head -c 10000 /dev/urandom > random.bin

RUN gcc -o client client.c

ENTRYPOINT ["./client"]
