#include "saharaCrypto.h"
#include "ns3/ipv4-header.h"
#include "saharaPacket.h"
#include <iostream>

  #include <cryptopp/aes.h> // For AES encryption
#include <cryptopp/base64.h> // For base64 encoding and decoding
#include <cryptopp/ccm.h> // For CCM mode
#include <cryptopp/cryptlib.h> // For CryptoPP::byte
#include <cryptopp/secblock.h> // For SecByteBlock
#include <cryptopp/osrng.h> // For AutoSeededRandomPool
#include <cryptopp/hex.h> // For hex encoding and decoding
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>




namespace ns3 {

NS_LOG_COMPONENT_DEFINE("saharaCrypto");



// Constructor: Initialize key and IV and set up encryption and decryption objects
CryptoModule::CryptoModule()
{
    std::string base64Key = "DjpQ0OYMhypYPGaQ2WsGvINivFXSKFgC";
    CryptoPP::Base64Decoder decoder;

    // Create a SecByteBlock to hold the decoded data
    CryptoPP::SecByteBlock key(32); // AES-256 uses a 32-byte key

    // Convert the base64-encoded key to a byte array
    decoder.Put((const CryptoPP::byte*)base64Key.data(), base64Key.length());
    decoder.MessageEnd();
    decoder.Get(key, key.size());
    
     // Copy the custom key and IV into member variables
    std::copy(key.begin(), key.end(), m_key);


    CryptoPP::byte fixedIV[CryptoPP::AES::BLOCKSIZE] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

    
    // Initialize AES encryption and decryption with the key and IV
    m_encryption.SetKeyWithIV(m_key, sizeof(m_key), fixedIV);
    m_decryption.SetKeyWithIV(m_key, sizeof(m_key), fixedIV);
   
}

// Encrypt a packet header
Ptr<Packet>
CryptoModule::EncryptHeader(Ptr<Packet> packet)
{
  
    // Retrieve the packet data
    uint32_t packetSize = packet->GetSize();
    uint8_t *data = new uint8_t[packetSize];
    packet->CopyData(data, packetSize);

    
     std::stringstream oss;
    for (uint32_t i = 0; i < packet->GetSize(); i++) {
        // Print each byte as a hexadecimal value
        oss << std::hex << std::uppercase << static_cast<int>(data[i]);
        // Add a space between bytes for readability
        if (i < packet->GetSize() - 1) {
            oss << " ";
        }
    }

    NS_LOG_DEBUG("Pre encryption content " << oss.str());


    // Encrypt the data
    m_encryption.ProcessData(data, data, packetSize);
     Ptr<Packet> ePacket = ns3::Create<ns3::Packet>(data, packetSize);
    NS_LOG_DEBUG("Packet to be encrypted size " << packet->GetSize());
    NS_LOG_DEBUG("Packet encrypted ");

    uint32_t packetSize3 = ePacket->GetSize();
    uint8_t *data2 = new uint8_t[packetSize];
    ePacket->CopyData(data2, packetSize);

    
     std::stringstream oss2;
    for (uint32_t i = 0; i < ePacket->GetSize(); i++) {
        // Print each byte as a hexadecimal value
        oss2 << std::hex << std::uppercase << static_cast<int>(data2[i]);
        // Add a space between bytes for readability
        if (i < ePacket->GetSize() - 1) {
            oss2 << " ";
        }
    }

    NS_LOG_DEBUG("Ecrypted content " << oss2.str());
   

    
    delete[] data;

    ePacket = DecryptHeader(ePacket);

    
    
    return ePacket;
    

}

// Decrypt a packet header
Ptr<Packet>
CryptoModule::DecryptHeader(Ptr<Packet> packet)
{

            uint32_t packetSize = packet->GetSize();
            uint8_t *encryptedData = new uint8_t[packetSize];
            packet->CopyData(encryptedData, packetSize);

            // Decrypt the data
            m_decryption.ProcessData(encryptedData, encryptedData, packetSize);

            NS_LOG_DEBUG("Packet decrypted");


            // Create a new packet with the decrypted data
            Ptr<Packet> decryptedPacket = Create<Packet>(encryptedData, packetSize);

             uint32_t packetSize2 = decryptedPacket->GetSize();
    uint8_t *data = new uint8_t[packetSize2];
    decryptedPacket->CopyData(data, packetSize2);

             std::stringstream oss;
    for (uint32_t i = 0; i < decryptedPacket->GetSize(); i++) {
        // Print each byte as a hexadecimal value
        oss << std::hex << std::uppercase << static_cast<int>(data[i]);
        // Add a space between bytes for readability
        if (i < decryptedPacket->GetSize() - 1) {
            oss << " ";
        }
    }

    NS_LOG_DEBUG("Decrypted content " << oss.str());

            // Clean up the temporary buffer
            delete[] encryptedData;

            uint32_t ePacketSize = decryptedPacket->GetSize();

            NS_LOG_DEBUG("Decrypted size " << ePacketSize);
            uint8_t *ePacketData = new uint8_t[ePacketSize];

           

            // Return the decrypted packet
            return decryptedPacket;
   
}

// Encrypt data using the AES encryption object
void CryptoModule::EncryptData(uint8_t* data, size_t size)
{
    m_encryption.ProcessData(data, data, size);
}

// Decrypt data using the AES decryption object
void CryptoModule::DecryptData(uint8_t* data, size_t size)
{
    m_decryption.ProcessData(data, data, size);
}

} // namespace ns3
