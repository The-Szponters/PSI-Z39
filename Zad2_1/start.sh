#!/bin/bash

echo "--- Buildig Server ---"
docker build -t z39_server_tcp_image ./Server
echo "--- Building Client ---"
docker build -t z39_client_tcp_image ./Client

docker rm -f z39_server_tcp 2>/dev/null || true
docker run -d --name z39_server_tcp --network z39_network z39_server_tcp_image

echo "--- Running 5 client in parallel ---"

for i in {1..5}
do
   docker run \
     --rm \
     --name z39_client_tcp_$i \
     --network z39_network \
     z39_client_tcp_image &
done

wait
echo "Tests Finished."