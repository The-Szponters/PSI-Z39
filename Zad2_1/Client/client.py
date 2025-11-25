import socket
import sys
import time

def run_client():
    SERVER_HOST = 'z39_server_app1'
    SERVER_PORT = 8000

    message = "Laboratorium PSI - Test wspolbieznosci"
    if len(sys.argv) > 1:
        message = sys.argv[1]

    client_socket = None

    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.settimeout(5)
    except socket.error as e:
        print(f"Error while creating socket: {e}")
        sys.exit(1)

    try:
        print(f"Connecting to {SERVER_HOST}:{SERVER_PORT}...")

        client_socket.connect((SERVER_HOST, SERVER_PORT))

        start_time = time.time()

        print(f"Sending message: '{message}'")
        client_socket.sendall(message.encode('utf-8'))

        data = client_socket.recv(1024)

        end_time = time.time()
        rtt_ms = (end_time - start_time) * 1000

        if data:
            response = data.decode('utf-8').strip()
            print(f"Success. RTT = {rtt_ms:.3f} ms.")
            print(f"Server Response: {response}")
        else:
            print("Server closed connection without response.")

    except socket.timeout:
        print("Timeout Error: Server did not respond in time.")
    except ConnectionRefusedError:
        print(f"Connection Refused: Is the server running on {SERVER_HOST}:{SERVER_PORT}?")
    except socket.error as e:
        print(f"Network Error: {e}")
    finally:
        if client_socket:
            client_socket.close()


if __name__ == '__main__':
    run_client()