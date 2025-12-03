import socket
import struct
import sys

# Konfiguracja
SERVER_PORT = 8000
MAX_BUFFER_SIZE = 1000000
MAX_FILE_SIZE = 20000

# Flagi
FLAG_START = 1
FLAG_IN_PROGRESS = 0
FLAG_FIN = 2
FLAG_ACK = 3

HEADER_FORMAT = '!IIH'
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)


def djb2_hash(data):
    hash_val = 5381
    for byte in data:
        hash_val = ((hash_val << 5) + hash_val) + byte
        hash_val = hash_val & 0xFFFFFFFFFFFFFFFF
    return hash_val


def run_server():
    try:
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        server_socket.bind(('', SERVER_PORT))
        print(f"UDP Server listens on {SERVER_PORT}")
    except socket.error as e:
        print(f"Error creating socket: {e}")
        sys.exit(1)

    expected_seq = 0
    started = False
    file_buf = bytearray()

    while True:
        try:
            data, client_addr = server_socket.recvfrom(MAX_BUFFER_SIZE)

            if len(data) < HEADER_SIZE:
                continue

            payload_size, seq_id, status = struct.unpack(HEADER_FORMAT, data[:HEADER_SIZE])
            payload = data[HEADER_SIZE:]

            send_ack = False

            if status == FLAG_START:
                started = True
                expected_seq = 0
                file_buf = bytearray()
                if payload_size > 0 and len(file_buf) + payload_size <= MAX_FILE_SIZE:
                    file_buf.extend(payload[:payload_size])
                send_ack = True

            elif status == FLAG_IN_PROGRESS:
                if not started:
                    send_ack = False
                elif seq_id == expected_seq:
                    if len(file_buf) + payload_size <= MAX_FILE_SIZE:
                        file_buf.extend(payload[:payload_size])
                        expected_seq += 1
                        send_ack = True
                    else:
                        send_ack = False
                elif seq_id < expected_seq:
                    send_ack = True
                else:
                    send_ack = False

            elif status == FLAG_FIN:
                if not started:
                    send_ack = False
                else:
                    if len(file_buf) + payload_size <= MAX_FILE_SIZE:
                        file_buf.extend(payload[:payload_size])

                    final_hash = djb2_hash(file_buf)
                    print(f"SERVER HASH: {final_hash}")

                    started = False
                    expected_seq = 0
                    file_buf = bytearray()
                    send_ack = True

            if send_ack:
                ack_packet = struct.pack(HEADER_FORMAT, 0, seq_id, FLAG_ACK)
                server_socket.sendto(ack_packet, client_addr)

        except Exception as e:
            print(f"Error handling packet: {e}")


if __name__ == '__main__':
    sys.stdout.reconfigure(line_buffering=True)
    run_server()
