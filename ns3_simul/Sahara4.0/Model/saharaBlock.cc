#include "saharaBlock.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>


using namespace std;
using namespace CryptoPP;


namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("saharaBlock");

    // Function to calculate the block's hash
void Block::calculateBlockHash() {

     std::ostringstream blockContent;

    for (const auto& tup : m_routingTable) {

        // Extract elements from the tuple
        Ipv4Address srcAddress = std::get<0>(tup);
        Ipv4Address dstAddress = std::get<1>(tup);
        uint16_t port1 = std::get<2>(tup);
        uint16_t port2 = std::get<3>(tup);
        uint16_t port3 = std::get<4>(tup);
        uint16_t port4 = std::get<5>(tup);
        uint16_t port5 = std::get<6>(tup);
        uint16_t port6 = std::get<7>(tup);
     
    
        blockContent << srcAddress << dstAddress << port1 << port2 << port3 << port4 << port5 << port6;
    }
        blockContent << m_previousHash;
  //  blockContent <<  m_chain.back().m_previousHash; // add hash of the previous block


    // Calculate SHA-256 hash of the block content using Crypto++
    CryptoPP::SHA256 hash;
    //std::string digest;

    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    m_hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(blockContent.str().data()), blockContent.str().size());

     std::ostringstream strDig;
        for (size_t i = 0; i < CryptoPP::SHA256::DIGESTSIZE; i++) {
            strDig << std::hex << std::setfill('0') << std::setw(2) << (int)digest[i];
        }
        m_blockHash = strDig.str();
    /*
    hash.Update(reinterpret_cast<const CryptoPP::byte*>(blockContent.str().data()), blockContent.str().size());
    digest.resize(hash.DigestSize());
    hash.Final(reinterpret_cast<CryptoPP::byte*>(&digest[0]));
    
    // Convert hash to a hex string
    CryptoPP::HexEncoder encoder;
    stringstream hashStringStream;
    encoder.Attach(new StringSink(hashStringStream));
    encoder.Put(reinterpret_cast<const CryptoPP::byte*>(digest.data()), digest.size());
    encoder.MessageEnd();
    m_blockHash = hashStringStream.str();
    */
}

// Add a new block with the routing table
void Blockchain::addBlock(const std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> tuples,  std::map<Ipv4Address, uint16_t> recvMap) {
    // Create a new block
    Block newBlock;
    newBlock.m_routingTable = tuples;
    m_mapIDRep = recvMap;
    // Set the previous hash
    if (!m_chain.empty()) {
        newBlock.m_previousHash = m_chain.back().m_blockHash;
    } else {
        newBlock.m_previousHash = "";  // Genesis block has no previous hash
    }

    // Calculate the block's hash
    newBlock.calculateBlockHash();

    // Add the new block to the blockchain
    m_chain.push_back(newBlock);
}

// Store the blockchain to a file
void Blockchain::storeToFile(std::ofstream &file) {
    //std::ofstream file(filename);
    if (file.is_open()) {
        //for (const auto& block : m_chain) {
            // Write the block data to the file
            file << "Block:\n";
            file << "Previous Hash: " << m_chain.back().m_previousHash << "\n";
            file << "Block Hash: " << m_chain.back().m_blockHash << "\n"; 
            file << "Routing Table:\n";
            for (const auto& tuple : m_chain.back().m_routingTable) {
            // Extract elements from the tuple
            const Ipv4Address& originIP = std::get<0>(tuple);
            const Ipv4Address& hop1IP = std::get<1>(tuple);
            uint16_t repO = m_mapIDRep[originIP];
            uint16_t repH = m_mapIDRep[hop1IP];
            uint16_t GPSO = std::get<4>(tuple);
            uint16_t GPSH = std::get<5>(tuple);
            uint16_t batteryO = std::get<6>(tuple);
            uint16_t batteryH = std::get<7>(tuple);
            
            
            
            // write elements
            file << originIP << ", " << hop1IP << ", " << repO << ", " << repH << ", " << GPSO <<", " << GPSH <<", " << batteryO <<", " << batteryH<< std::endl;
        }
            
            file << "End of Block\n";
      //  }
       
    } else {
        cerr << "Unable to open file for writing.\n";
    }
    
}



}
