
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

 */

class SaharaHeader : public Header
{
  public:
    enum MessageType
    {
        HELLO_MESSAGE = 1,
        SET_BF = 2,
        SET_ACK = 3,
        SEND_MISSING_C2P = 4,
        SEND_MISSING_P2C = 5
        
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
    

    
    void SetOriginIP(Ipv4Address orIP);
    void SetHop1IP(Ipv4Address h1);
    void SetReputation_O(uint16_t rep);
    void SetGPS_O(uint16_t gps);
    void SetBattery_O(uint16_t bat);
    void SetReputation_H(uint16_t rep);
    void SetGPS_H(uint16_t gps);
    void SetBattery_H(uint16_t bat);

    // for set reconciliation
    void SetParentIP(Ipv4Address parentIP);
    void SetChildIP(Ipv4Address childIP);
    void SetBF(std::vector<bool> BF);
    void SetMissingTuples(std::vector<std::tuple<Ipv4Address,Ipv4Address,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t>> missingTuples);
  
    
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
    
};

} // namespace Sahara
} // namespace ns3

#endif /* SAHARA_PACKET_H */
