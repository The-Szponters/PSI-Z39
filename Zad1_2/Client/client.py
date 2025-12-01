import socket
import sys
import os
import struct


CHUNK_SIZE = 100
N_CHUNKS = 100

# Template: PayloadSize (4B), SeqID (4B), StatusFlag (2B) = 10 Bytes
HEADER_FORMAT = '!IIH'
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
START_TRANSMISSION = 1
TRANSMISSION_IN_PROGRESS = 0
END_TRANSMISSION = 2
ACK_FLAG = 3


def djb2_hash(data):
    hash_val = 5381
    for byte in data:
        hash_val = ((hash_val << 5) + hash_val) + byte
        hash_val = hash_val & 0xFFFFFFFFFFFFFFFF
    return hash_val


def handle_file(filepath):
    hash = ""
    chunks = []
    with open(filepath, "rb") as f:
        hash = djb2_hash(f.read())
        f.seek(0)
        for i in range(N_CHUNKS):
            chunk = f.read(CHUNK_SIZE)
            chunks.append(chunk)
    return hash, chunks


def prepare_header(payload_size, seq_id, status_flag):
    return struct.pack(HEADER_FORMAT, payload_size, seq_id, status_flag)


def run_client():
    SERVER_HOST = 'z39_server_app2'
    SERVER_PORT = 8000

    hash, chunks = handle_file(filepath="./random.bin")
    print(f"CLIENT HASH: {hash}")

    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        client_socket.settimeout(2)
    except socket.error as e:
        print(f"Error while creating socket: {e}")
        sys.exit(1)

    while True:
        try:
            client_socket.sendto(prepare_header(0, 0, START_TRANSMISSION), (SERVER_HOST, SERVER_PORT))
            ack_data, _ = client_socket.recvfrom(1024)

            if len(ack_data) >= HEADER_SIZE:
                ack_flag = struct.unpack(HEADER_FORMAT, ack_data[:HEADER_SIZE])[2]

                if ack_flag == ACK_FLAG:
                    print(f"INFO: Established connection with {SERVER_HOST}:{SERVER_PORT}")
                    break
                else:
                    print(f"ERROR: Got invalid ACK flag: {ack_flag}")
            else:
                print("ERROR: Incomplete header received")

        except socket.timeout:
            print("ERROR: Timeout Error.")
        except socket.error as e:
            print(f"ERROR: Network Error: \n{e}")

    for i, chunk in enumerate(chunks):
        payload = prepare_header(CHUNK_SIZE, i, TRANSMISSION_IN_PROGRESS) + chunk
        while True:
            try:
                client_socket.sendto(payload, (SERVER_HOST, SERVER_PORT))
                ack_data, _ = client_socket.recvfrom(1024)

                if len(ack_data) >= HEADER_SIZE:
                    ack_seq = struct.unpack(HEADER_FORMAT, ack_data[:HEADER_SIZE])[1]
                    ack_flag = struct.unpack(HEADER_FORMAT, ack_data[:HEADER_SIZE])[2]

                    if ack_seq == i and ack_flag == ACK_FLAG:
                        print(f"INFO: Got ACK form {i}th chunk")
                        break
                    else:
                        print(f"ERROR: Got invalid ACK SeqID (expected {i}, got {ack_seq})")
                else:
                    print("ERROR: Incomplete header received")

            except socket.timeout:
                print("Timeout Error.")
            except socket.error as e:
                print(f"Network Error: \n{e}")

    while True:
        try:
            client_socket.sendto(prepare_header(0, 0, END_TRANSMISSION), (SERVER_HOST, SERVER_PORT))
            ack_data, _ = client_socket.recvfrom(1024)

            if len(ack_data) >= HEADER_SIZE:
                ack_flag = struct.unpack(HEADER_FORMAT, ack_data[:HEADER_SIZE])[2]

                if ack_flag == ACK_FLAG:
                    print(f"INFO: Finalized connection with {SERVER_HOST}:{SERVER_PORT}")
                    break
                else:
                    print("ERROR: Got invalid ACK SeqID")
            else:
                print("ERROR: Incomplete header received")

        except socket.timeout:
            print("ERROR: Timeout Error.")
        except socket.error as e:
            print(f"ERROR: Network Error: \n{e}")

    client_socket.close()


if __name__ == '__main__':
    run_client()
