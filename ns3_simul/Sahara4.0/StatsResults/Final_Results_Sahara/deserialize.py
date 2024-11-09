import binascii

def hex_to_bits(hex_string):
    # Remove any whitespace from the hex string
    hex_string = ''.join(hex_string.split())
    
    # Convert hex string to bytes
    byte_data = binascii.unhexlify(hex_string)
    
    # Convert bytes to bits
    bits = ''.join(format(byte, '08b') for byte in byte_data)
    
    return bits

# Use the function

hex_data = "00 00 00 00 00 08 00 00 00 00 00 00 00 00 00 20 80 00 00 40 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 08 00 00 00 20 00 00 04 00 01 20 00 00 00 00 00 00 01 00 00 20 00 00 00 01 00 00 00 00 08 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00 00 00 00 04 00 00 00 00 00 00 00 00 00 00 11 00 00 00 00 00 00 00 20 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 08 00 00 00 00 20 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 20 00 00 00 00 00 20 00 00 00 00 00 00 00 00 00 00 08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 20 00 20 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 00 00 00 00 01 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 08 00 00 00 00 00 00 00 00 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"
bits = hex_to_bits(hex_data)

print("Bit representation:")
print(bits)

# Print in groups of 8 for readability
print("\nBit representation (grouped by bytes):")
for i in range(0, len(bits), 8):
    print(bits[i:i+8], end=' ')
    
    if (i+8) % 64 == 0:  # New line every 8 bytes for better readability
        print()