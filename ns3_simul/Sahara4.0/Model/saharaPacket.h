
#ifndef SAHARA_PACKET_H
#define SAHARA_PACKET_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

#include <iostream>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>


namespace ns3
{
namespace sahara
{
/** SaharaPacket structure
 |      0        |      1        |      2        |       3       |
  0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                            originIP                           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                             hop1IP                            |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                   reputation_O | reputation_H                 |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                          GPS_O | GPS_H                        |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
 |                      battery_O | battery_H                    |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 vote_O, vote_H,
 signO
 signH


 */



class SaharaHeader : public Header
{
 
  public:


    enum MessageType
    {
        HELLO_MESSAGE = 1, // flooding 
        // set reconciliation: 
        SEND_MISSING_C2P = 2, // sending missing tuples from child to parent
        SEND_MISSING_P2C = 3, // send missing tuples from parent to child
        SR_HELLO = 4, // set reconciliation hello messages 
        ROOT_SR_HELLO = 5, // root hello message (first messahe from root that starts set reconciliation)
        SEND_MISSING_P2C_ACK = 6, // child confirms with an ack to have received missing tuples from parent 
        ASK_BF = 7, // child asks to parent his bloom filter 
        SEND_BF = 8, // parent sends its bloom filter to child
        ENCRYPTED = 9,
        // these two below are only used for the Sahara SR Dynamic
        ASK_BF_P2C = 10,
        SEND_BF_C2P = 11

    };

    enum class VoteState : uint8_t {
        Negative = 0,
        Positive = 1,
        Undefined = 2
    };

  
    struct Vote {
        Ipv4Address EvaluatedIP; // L'indirizzo IP di chi subito dato il voto
        SaharaHeader::VoteState vote;         
    };

   struct VotePacket {
    VotePacket() = default;  // Add this line for default constructor
    VotePacket(Ipv4Address evalIP, const std::list<Vote>& vtList, const std::string& sig)
        : evaluatorIP(evalIP), votes(vtList), signature(sig) {}

    Ipv4Address evaluatorIP;
    std::list<Vote> votes;
    std::string signature;
};


    

    /*
    SaharaHeader(Ipv4Address originIP = Ipv4Address(), Ipv4Address hop1IP = Ipv4Address(), 
                        uint16_t reputation_O = 0,uint16_t reputation_H = 0, uint16_t GPS_O = 0, uint16_t GPS_H=0, uint16_t battery_O = 0,
                        uint16_t battery_H = 0);

    SaharaHeader(Ipv4Address childIP, bool only);

    SaharaHeader(Ipv4Address parentIP, std::vector<bool> bloomFilter);
    */


    SaharaHeader();

    ~SaharaHeader() override;
    
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;
    void SetMessageType(MessageType messageType);
    MessageType GetMessageType() 
    {
        return m_messageType;
    }
    void WriteStringToBuffer(ns3::Buffer::Iterator &i, const std::string &str) const;
    std::string ReadStringFromBuffer(ns3::Buffer::Iterator &i);

    void WriteVotesIntoBuffer(ns3::Buffer::Iterator &i) const;
    void ReadVotesFromBuffer(ns3::Buffer::Iterator &i);

    void WriteVotesIntoBuffer2(ns3::Buffer::Iterator &i) const;
    void ReadVotesFromBuffer2(ns3::Buffer::Iterator &i, uint16_t sizeMissingVoteList);

    
    void SetOriginIP(Ipv4Address orIP);
    void SetHop1IP(Ipv4Address h1);
    void SetReputation_O(uint16_t rep);
    void SetGPS_O(uint16_t gps);
    void SetBattery_O(uint16_t bat);
    void SetReputation_H(uint16_t rep);
    void SetGPS_H(uint16_t gps);
    void SetBattery_H(uint16_t bat);
    void SetRTDim(uint16_t rtDim); // number of tuples
    
    void SetVote_O(VoteState voteO);
    void SetVote_H(VoteState voteH);
    void SetSign_O(std::string signO);
    void SetSign_H(std::string signH);


    // this is for message encryption
    void SetEncryptedPacketSize(u_int32_t encrSize);


    // for set reconciliation
    void SetParentIP(Ipv4Address parentIP);
    void SetChildIP(Ipv4Address childIP);
    void SetBF(std::vector<bool> BF);
    void SetMissingTuples(std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> missingTuples);


    // for votes
    void SetVotes(SaharaHeader::VotePacket votePacket);
    void SetListVotesIP(std::vector<Ipv4Address> votesIPList);
    void SetMissingVotePackets(const std::vector<ns3::sahara::SaharaHeader::VotePacket>& votePackets);
    
    Ipv4Address GetOriginIP();
    Ipv4Address GetHop1IP();
    uint16_t GetReputation_O();
    uint16_t GetGPS_O();
    uint16_t GetBattery_O();
    uint16_t GetReputation_H();
    uint16_t GetGPS_H();
    uint16_t GetBattery_H();
    Ipv4Address GetChildIP();
    std::vector<bool> GetBloomFilter();
    Ipv4Address GetParentIP();
    uint16_t GetSizeBF();
    std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> GetMissingTuples();
    uint16_t GetRTdim();
    VoteState GetVote_O();
    VoteState GetVote_H();
    std::string GetSign_O();
    std::string GetSign_H();
    
    // for votes
    VotePacket GetVotes();
    std::vector<Ipv4Address> GetListVotesIP();
    const std::vector<ns3::sahara::SaharaHeader::VotePacket>& GetMissingVotePackets() const;

    


  private:
    Ipv4Address m_originIP;
    Ipv4Address m_hop1IP;
    uint16_t m_reputation_O;
    uint16_t m_reputation_H;
    uint16_t m_GPS_O;
    uint16_t m_GPS_H;
    uint16_t m_battery_O;
    uint16_t m_battery_H;
    MessageType m_messageType;
    Ipv4Address m_childIP;
    Ipv4Address m_parentIP;
    std::vector<bool> m_bloomFilter;
    u_int16_t sizeBF;
    std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> m_missing_tuples;
    u_int16_t m_rtDim;
    VoteState m_vote_O = VoteState::Undefined;
    VoteState m_vote_H = VoteState::Undefined;
    std::string m_sign_O;
    std::string m_sign_H;
    std::string m_sign;

    // encryption
    uint32_t m_encryptedSize = 0;


    // for votes 
    SaharaHeader::VotePacket m_votes;   
    std::vector<Ipv4Address> m_votesIPList;
    std::vector<ns3::sahara::SaharaHeader::VotePacket> m_missing_VotePackets;

    uint32_t GetSizeVotesList() const;
     uint32_t GetSizeVotesList2() const;
    
};

} // namespace Sahara
} // namespace ns3

#endif /* SAHARA_PACKET_H */
