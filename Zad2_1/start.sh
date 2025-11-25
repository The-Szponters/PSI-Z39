#!/bin/bash

echo "--- Budowanie Serwera ---"
docker build -t z39_server_tcp_image ./Server
docker rm -f z39_server_tcp 2>/dev/null || true

docker run -d \
  --name z39_server_tcp \
  --network z39_network \
  z39_server_tcp_image

docker build -t z39_client_tcp_image ./Client

for i in {1..5}
do
   CLIENT_NAME="z39_client_tcp_$i"
   docker rm -f $CLIENT_NAME 2>/dev/null || true

   docker run \
     --name $CLIENT_NAME \
     --network z39_network \
     z39_client_tcp_image "Wiadomosc_od_klienta_$i" &
done

wait
echo "Testy zakończone."
