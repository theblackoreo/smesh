import secrets

def generate_16_byte_key():
    key = secrets.token_bytes(16)
    return key

key = generate_16_byte_key()
print(key)
