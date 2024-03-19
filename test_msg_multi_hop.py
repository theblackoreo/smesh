import socket


port = 12345  # Choose an appropriate port number

# Create a socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Connect to ipC via ipB
    client_socket.connect(("192.168.1.0", port))

    # Send a message from ipA to ipC via ipB
    message_to_C = "Hello from ipA to ipC via ipB"
    client_socket.sendall(message_to_C.encode())
    print("Message sent from ipA to ipC via ipB")

    # Close the connection with ipC
    client_socket.close()

except Exception as e:
    print("An error occurred:", e)
