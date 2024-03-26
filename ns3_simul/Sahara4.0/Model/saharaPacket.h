
#ifndef SAHARA_PACKET_H
#define SAHARA_PACKET_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

#include <iostream>

namespace ns3
{
namespace sahara
{
/**
 |      0        |      1        |      2        |       3       |
  0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                            originIP                           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                             hop1IP                            |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                   reputation | GPS | Battery                  |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

 */

class SaharaHeader : public Header
{
  public:
   
    SaharaHeader(Ipv4Address originIP = Ipv4Address(), Ipv4Address hop1IP = Ipv4Address(), 
                        uint32_t reputation = 0,uint32_t GPS = 0, uint32_t battery = 0);
    ~SaharaHeader() override;
   
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;

    
    void SetOriginIP(Ipv4Address orIP)
    {
        m_originIP = orIP;
    }

    void SetHop1IP(Ipv4Address h1)
    {
        m_hop1IP = h1;
    }


    void SetReputation(uint32_t rep)
    {
        m_reputation = rep;
    }

     void SetGPS(uint32_t gps)
    {
        m_GPS = gps;
    }

     void SetBattery(uint32_t bat)
    {
       m_battery = bat;
    }

  private:
    Ipv4Address m_originIP;
    Ipv4Address m_hop1IP;
    uint32_t m_reputation;
    uint32_t m_GPS; 
    uint32_t m_battery;
};

/*
static inline std::ostream&
operator<<(std::ostream& os, const SaharaHeader& packet)
{
    packet.Print(os);
    return os;
}
*/
} // namespace Sahara
} // namespace ns3

#endif /* SAHARA_PACKET_H */
