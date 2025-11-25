import socket
import time

SERVER_HOST = 'z39_server_tcp'
SERVER_PORT = 8000
MESSAGE = "Test PSI Message"
DELAY = 5


def djb2_hash(text):
    hash_val = 5381
    for char in text:
        hash_val = ((hash_val << 5) + hash_val) + ord(char)
        hash_val = hash_val & 0xFFFFFFFFFFFFFFFF
    return hash_val


def run_client():
    client_socket = None
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        print(f"[Client] Connecting to {SERVER_HOST}:{SERVER_PORT}...")
        client_socket.connect((SERVER_HOST, SERVER_PORT))
        print("[Client] Connected.")

        if DELAY > 0:
            print(f"[Client] Simulating 'slow client'. Sleeping for {DELAY}s...")
            time.sleep(DELAY)
            print("[Client] Resuming work after sleep.")

        print(f"[Client] Sending: '{MESSAGE}'")
        client_socket.sendall(MESSAGE.encode('utf-8'))

        data = client_socket.recv(1024)
        response = data.decode('utf-8').strip()
        print(f"[Client] Server response: {response}")

        if "Hash:" in response:
            try:
                server_hash = int(response.split(':')[-1].strip())
                local_hash = djb2_hash(MESSAGE)

                print(f"[Test A] Local hash: {local_hash}")
                print(f"[Test A] Server hash: {server_hash}")

                if local_hash == server_hash:
                    print("[Test A] SUCCESS: Hashes are identical.")
                else:
                    print("[Test A] ERROR: Hashes differ!")
            except ValueError:
                print("[Test A] Error parsing hash number.")
        else:
            print("[Test A] Failed to parse response for verification.")

    except Exception as e:
        print(f"Error: {e}")
    finally:
        if client_socket:
            client_socket.close()


if __name__ == "__main__":
    run_client()
