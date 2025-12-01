import socket
import sys
import os


def create_payload(size):
    payload = os.urandom(size)
    return payload


def djb2_hash(text):
    hash_val = 5381
    for char in text:
        hash_val = ((hash_val << 5) + hash_val) + ord(char)
        hash_val = hash_val & 0xFFFFFFFFFFFFFFFF
    return hash_val


def handle_file(filepath):
    hash = ""

    with open(filename, "rb")
      



def run_client():
    SERVER_HOST = 'z39_server_app2'
    SERVER_PORT = 8000

    handle_file(filepath="random.bin,")

    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        client_socket.settimeout(2)
    except socket.error as e:
        print(f"Error while creating socket: {e}")
        sys.exit(1)
    try:
        while True:
            client_socket.sendto("ds", (SERVER_HOST, SERVER_PORT))
            data, server_address = client_socket.recvfrom(64)
    except socket.timeout:
        print(f"Payload size: {len(payload)} B, Timeout Error.")
    except socket.error as e:
        print(f"Payload size: {len(payload)} B, Network Error: {e}")

    client_socket.close()


if __name__ == '__main__':
    run_client()
