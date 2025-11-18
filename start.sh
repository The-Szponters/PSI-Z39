#!/bin/bash

docker build -t z39_server_app1_image ./Server
docker rm -f z39_server_app1 2>/dev/null || true
docker run -d \
  --name z39_server_app1 \
  --network z39_network \
  z39_server_app1_image

docker build -t z39_client_app1_image ./Client
docker rm -f z39_client_app1 2>/dev/null || true
docker run \
  --name z39_client_app1 \
  --network z39_network \
  z39_client_app1_image
