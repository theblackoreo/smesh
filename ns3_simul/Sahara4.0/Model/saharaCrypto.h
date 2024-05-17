#ifndef CRYPTO_MODULE_H
#define CRYPTO_MODULE_H

#include "ns3/packet.h"
#include "ns3/header.h"
#include <crypto++/aes.h>
#include <crypto++/cryptlib.h>
#include <crypto++/modes.h>
#include <crypto++/osrng.h>
#include "saharaPacket.h"


namespace ns3 {

class CryptoModule
{
public:

    CryptoModule();

    Ptr<Packet> EncryptHeader(Ptr<Packet> packet);

    Ptr<Packet> DecryptHeader(Ptr<Packet> packet);

private:
    // Key and IV for encryption and decryption
    CryptoPP::byte m_key[CryptoPP::AES::DEFAULT_KEYLENGTH];
    CryptoPP::byte m_iv[CryptoPP::AES::BLOCKSIZE];

    // AES encryption and decryption objects
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption m_encryption;
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption m_decryption;

    // Helper methods for encryption and decryption
    void EncryptData(uint8_t* data, size_t size);
    void DecryptData(uint8_t* data, size_t size);
};

} // namespace ns3

#endif // CRYPTO_MODULE_H
