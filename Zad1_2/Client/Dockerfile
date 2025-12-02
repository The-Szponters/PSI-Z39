FROM python:3

RUN apt-get update && apt-get install -y iproute2

COPY client.py /
COPY random.bin /
COPY generate_file.sh /

RUN chmod +x /generate_file.sh && /generate_file.sh

WORKDIR /
ENTRYPOINT [ "/bin/bash", "-c" ]