#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <vector>
#include <tuple>
#include <string>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include "ns3/ipv4-address.h"
#include <limits>
#include <fstream>


namespace ns3
{


class Block {
public:
    std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> m_routingTable;
    std::string m_previousHash;
    std::string m_blockHash;
    CryptoPP::SHA256 m_hash; 

    // Function to calculate the block's hash
    void calculateBlockHash();
};

// Define the blockchain
class Blockchain {
public:

    std::vector<Block> m_chain;

    // Add a new block with the routing table
    void addBlock(const std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> tuples);

    // Store the blockchain to a file
    void storeToFile(std::ofstream &file);

    // Restore the blockchain from a file
    void restoreFromFile(const std::string& filename);
};


}

#endif