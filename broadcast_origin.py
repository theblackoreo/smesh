import socket
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import padding
from base64 import b64encode, b64decode
import msg_broadcast_struct_pb2 as msg_struct
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

# create a socket and send message in broadcast using UDP
def send_broadcast(msg, port):
                sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
                
                #broacast address
                indirizzo_broadcast = '192.168.1.255'
                
                dest = (indirizzo_broadcast, port)
                
                sock.sendto(msg.encode(), dest)
                print("Message sent in broadcast")
                sock.close()

#decrypt message received using symmetric shared key
def decrypt_message(encrypted_message, key):

    ciphertext = b64decode(encrypted_message)

    cipher = Cipher(algorithms.AES(key), modes.ECB(), backend=default_backend())
    decryptor = cipher.decryptor()

    padded_data = decryptor.update(ciphertext) + decryptor.finalize()

    #padding removal
    unpadder = padding.PKCS7(128).unpadder()
    message = unpadder.update(padded_data)
    message += unpadder.finalize()

    return message

def encrypt_message(message, key):
    
    #adding padding
    padder = padding.PKCS7(128).padder()
    padded_data = padder.update(message) + padder.finalize()

    cipher = Cipher(algorithms.AES(key), modes.ECB(), backend=default_backend())
    encryptor = cipher.encryptor()
    ciphertext = encryptor.update(padded_data) + encryptor.finalize()

    return b64encode(ciphertext).decode('utf-8')

def receive_broadcast(port, key, my_ip):
    
    #UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    sock.bind(('192.168.1.255', port))
    
    print("Listeing for broadcast messages...")

    msg_id_processed = []
    
    while True:
        msg, address = sock.recvfrom(1024)
        

        msg_decrypted = decrypt_message(msg.decode(), key)

        # Assuming serialized_data contains the serialized binary data
        # Deserialize the binary data into a SenderInfo message object
        rcv_msg = msg_struct.SenderInfo()
        rcv_msg.ParseFromString(msg_decrypted)

        # only to test rebroadcasting
        if ((rcv_msg.ip_sender == "192.168.1.3" and my_ip == "192.168.1.1") or (rcv_msg.ip_sender == "192.168.1.1" and my_ip == "192.168.1.0")):
           
            if(rcv_msg.msg_type == 1 and rcv_msg.msg_id not in msg_id_processed):
                print("Message received from {}: {}".format(address, msg.decode()))
                
                # add the msg_id to the list of processed messages to avoid processing the same message multiple times

                msg_id_processed.append(rcv_msg.msg_id)

                # Now you can access the fields of the received message
                origin_ip = rcv_msg.ip_origin
                sender_ip = rcv_msg.ip_sender
                reputation_score = rcv_msg.reputation
                battery_percentage = rcv_msg.battery
                gps_location = rcv_msg.GPS

                if sender_ip != my_ip:
                

                    # You can then use these values as needed
                    print("Message ID:", rcv_msg.msg_id)
                    print("Origin ID:", origin_ip)
                    print("Sender ID:", sender_ip)
                    print("Reputation Score:", reputation_score)
                    print("Battery Percentage:", battery_percentage)
                    print("GPS location:", gps_location)

                    rcv_msg.ip_sender = my_ip

                    # Serialize the message to binary format
                    serialized_data = rcv_msg.SerializeToString()

                    encrypted_message = encrypt_message(serialized_data, key)

                    port = 12345

                    send_broadcast(encrypted_message, port)
                



# Get the IP address of wlan0
wlan0_ip = get_ip_address('wlan0')
if wlan0_ip:
    print("IP address of wlan0:", wlan0_ip)
else:
    print("Failed to retrieve IP address of wlan0")

port = 12345

# Avvia la ricezione dei messaggi in broadcast sulla porta specificata
# 16 byte key
key = b'\xec\xb97x\x08p{\x91\x86\xf6`N\xfe9\x81\xf0'
receive_broadcast(port, key, wlan0_ip)

# msg types: 1 = origin_update

# if origin_update:
#     # 1) save it in own routing table
#     # 2) forward it to all neighbors in broadcast attacking the id of the sender




