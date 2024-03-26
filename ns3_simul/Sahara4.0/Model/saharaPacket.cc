
#include "saharaPacket.h"

#include "ns3/address-utils.h"
#include "ns3/packet.h"

namespace ns3
{
namespace sahara
{

NS_OBJECT_ENSURE_REGISTERED(SaharaHeader);

SaharaHeader::SaharaHeader(Ipv4Address originIP, Ipv4Address hop1IP, 
                        uint32_t reputation,uint32_t GPS, uint32_t battery)
    : m_originIP(originIP),
      m_hop1IP(hop1IP),
      m_reputation(reputation),
      m_GPS(GPS),
      m_battery(battery)
{
}

SaharaHeader::~SaharaHeader()
{
}

TypeId
SaharaHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::sahara::SaharaHeader")
                            .SetParent<Header>()
                            .SetGroupName("Sahara")
                            .AddConstructor<SaharaHeader>();
    return tid;
}

TypeId
SaharaHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
SaharaHeader::GetSerializedSize() const
{
    return 12;
}

void
SaharaHeader::Serialize(Buffer::Iterator i) const
{
    WriteTo(i, m_originIP);
    WriteTo(i, m_hop1IP);
    i.WriteHtonU8(m_reputation);
    i.WriteHtonU8(m_GPS);
    i.WriteHtonU8(m_battery);
}

uint32_t
SaharaHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    ReadFrom(i, m_originIP);
    ReadFrom(i, m_hop1IP);
    m_reputation = i.ReadNtohU8();
    m_GPS = i.ReadNtohU8();
    m_battery = i.ReadNtohU8();

    uint32_t dist = i.GetDistanceFrom(start);
    NS_ASSERT(dist == GetSerializedSize());
    return dist;
}

/*
void
SaharaHeader::Print(std::ostream& os) const
{
    os << "DestinationIpv4: " << m_dst << " Hopcount: " << m_hopCount
       << " SequenceNumber: " << m_dstSeqNo;
}*/
} // namespace Sahara
} // namespace ns3
