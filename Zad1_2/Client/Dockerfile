FROM python:3

RUN apt-get update && apt-get install -y iproute2

COPY client.py /

RUN head -c 10000 /dev/urandom > random.bin

WORKDIR /

ENTRYPOINT [ "/bin/bash", "-c" ]