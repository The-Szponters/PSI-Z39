import socket
import sys
import time
import os
import struct


def create_payload(size):
    payload = os.urandom(size)
    return payload


def run_client():
    SERVER_HOST = 'z39_server_app1'
    SERVER_PORT = 8000

    current_size = 1

    test_results = []

    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        client_socket.settimeout(2)
    except socket.error as e:
        print(f"Error while creating socket: {e}")
        sys.exit(1)

    try:
        while True:
            payload = create_payload(current_size)
            start_time = time.time()
            client_socket.sendto(payload, (SERVER_HOST, SERVER_PORT))
            data, server_address = client_socket.recvfrom(64)
            print(f"Obtained response from the server: {server_address}")
            end_time = time.time()
            rtt_ms = (end_time - start_time) * 1000
            test_results.append((len(payload), rtt_ms))
            print(f"Size: {len(payload)} B, Success. RTT = {rtt_ms:.3f} ms. Response: {data}")
            current_size *= 2
    except socket.timeout:
        print(f"Size: {len(payload)} B, Timeout Error.")
    except socket.error as e:
        print(f"Size: {len(payload)} B, Network Error: {e}")

    print("\n--- Starting binary search for maximum payload size ---")
    left = current_size // 2
    right = current_size
    curr = 0
    while (left <= right):
        curr = (left + right) // 2
        payload = create_payload(curr)
        try:
            client_socket.sendto(payload, (SERVER_HOST, SERVER_PORT))
            data, server_address = client_socket.recvfrom(64)
            left = curr + 1
        except socket.timeout:
            print(f" Size: {len(payload)} B, Timeout Error.")
            right = curr - 1
        except socket.error as e:
            print(f"Size: {len(payload)} B, Network Error: {e}")
            right = curr - 1

    print(f"Max size : {len(payload)} B")
    client_socket.close()

    print("\n--- Size, RTT ---")
    for size, rtt in test_results:
        print(f"{size},{rtt:.3f}")
    print("------------------------------------------")


if __name__ == '__main__':
    run_client()
