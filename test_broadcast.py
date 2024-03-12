import socket
import random
import msg_broadcast_struct_pb2 as msg
import time

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import padding
from base64 import b64encode, b64decode
import fcntl
import struct

def get_ip_address(ifname):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        ip_address = socket.inet_ntoa(fcntl.ioctl(
            sock.fileno(),
            0x8915,  # SIOCGIFADDR
            struct.pack('256s', ifname.encode('utf-8')[:15])
        )[20:24])
        return ip_address
    except Exception as e:
        print("Error:", e)
        return None

def generate_random_number():
    return random.randint(0, 100000)

def encrypt_message(message, key):
    
    # message padding
    padder = padding.PKCS7(128).padder()
    padded_data = padder.update(message) + padder.finalize()

    cipher = Cipher(algorithms.AES(key), modes.ECB(), backend=default_backend())
    encryptor = cipher.encryptor()

    ciphertext = encryptor.update(padded_data) + encryptor.finalize()

    return b64encode(ciphertext).decode('utf-8')



# 16 byte key
key = b'\xec\xb97x\x08p{\x91\x86\xf6`N\xfe9\x81\xf0'

def invia_broadcast(messaggio, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    
    broadcast_address = '192.168.1.255'
    
    dest = (broadcast_address, port)
    
    sock.sendto(messaggio.encode(), dest)
    

    # Chiudi il socket dopo l'invio del messaggio
    sock.close()

def send_update(my_ip):

    # Get the hostname of the device
    hostname = socket.gethostname()

    # Get the IP address corresponding to the hostname

    print("My IP address:", my_ip)
    # Sample data
    sender_info = msg.SenderInfo()
    sender_info.ip_origin = my_ip
    sender_info.ip_sender = my_ip
    sender_info.msg_id = generate_random_number()
    sender_info.msg_type = 1

    sender_info.reputation = 85
    sender_info.battery = 70
    sender_info.GPS = 1233442

    # Serialize the message to binary format
    serialized_data = sender_info.SerializeToString()

    # Crittografare il messaggio
    encrypted_message = encrypt_message(serialized_data, key)
    print("Encrypted message:", encrypted_message)

    port = 12345

    # Invia il messaggio in broadcast sulla porta specificata
    invia_broadcast(encrypted_message, port)



# Get the IP address of wlan0
wlan0_ip = get_ip_address('wlan0')
if wlan0_ip:
    print("IP address of wlan0:", wlan0_ip)
else:
    print("Failed to retrieve IP address of wlan0")

while True:
        # Genera un tempo casuale tra 1 e 10 secondi
        random_time = random.uniform(1, 10)
        
        # Attendi il tempo casuale prima di eseguire la funzione
        time.sleep(random_time)
        
        # Esegui la funzione
        send_update(wlan0_ip)






