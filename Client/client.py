import socket
import sys
import time
import os
import struct


def create_payload(size):
    header = struct.pack('!I', size)
    payload = header + os.urandom(size)
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
        print(f"Błąd tworzenia gniazda: {e}")
        sys.exit(1)

    try:
        while True:
            payload = create_payload(current_size)
            start_time = time.time()
            client_socket.sendto(payload, (SERVER_HOST, SERVER_PORT))
            data, server_address = client_socket.recvfrom(64)
            print(f"Otrzymano odpowiedź od serwera: {server_address}")
            end_time = time.time()
            rtt_ms = (end_time - start_time) * 1000
            test_results.append((current_size, rtt_ms))
            print(f"Rozmiar: {current_size} B, Sukces. RTT = {rtt_ms:.3f} ms. Odp. ACK: {len(data)} B.")
            current_size *= 2
    except socket.timeout:
        print(f"Rozmiar: {current_size} B, Błąd: Timeout. Prawdopodobny limit osiągnięty.")
    except socket.error as e:
        print(f"Rozmiar: {current_size} B, Błąd sieci: {e}")

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
            print(f"Rozmiar: {curr} B, Błąd: Timeout.")
            right = curr - 1
        except socket.error as e:
            print(f"Rozmiar: {curr} B, Błąd sieci: {e}")
            right = curr - 1

    print(f"Maksymalny rozmiar : {right} B")
    client_socket.close()

    print("\n--- Zestawienie ---")
    for size, rtt in test_results:
        print(f"{size},{rtt:.3f}")
    print("------------------------------------------")


if __name__ == '__main__':
    run_client()
