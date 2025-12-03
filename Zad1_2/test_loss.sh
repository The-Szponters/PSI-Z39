#!/bin/bash

echo "--- BUILDING IMAGES ---"
docker build -t z39_server_app2_image ./Server
docker build -t z39_client_app2_image ./Client

docker rm -f z39_server_app2 2>/dev/null || true
docker rm -f z39_client_app2 2>/dev/null || true

echo "--- STARTING SERVER ---"
docker run -d \
  --name z39_server_app2 \
  --network z39_network \
  z39_server_app2_image

sleep 2

echo "--- STARTING CLIENT WITH 30% PACKET LOSS ---"
docker run \
  --name z39_client_app2 \
  --network z39_network \
  --cap-add=NET_ADMIN \
  z39_client_app2_image \
  "tc qdisc add dev eth0 root netem loss 30% && ./client"

echo " "
echo "=== SERVER LOGS (CHECK FOR HASH, 30% LOSS) ==="
docker logs z39_server_app2

echo " "
echo "--- STARTING CLIENT WITH 60% PACKET LOSS ---"
docker rm -f z39_client_app2 2>/dev/null || true

docker run \
  --name z39_client_app2 \
  --network z39_network \
  --cap-add=NET_ADMIN \
  z39_client_app2_image \
  "tc qdisc add dev eth0 root netem loss 60% && ./client"

echo " "
echo "=== SERVER LOGS (CHECK FOR HASH, 60% LOSS) ==="
docker logs z39_server_app2