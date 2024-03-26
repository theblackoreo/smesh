#include "SaharaHeader.h"
#include "ns3/buffer.h"

namespace ns3 {
namespace sahara {

// PacketHeader implementation

PacketHeader::PacketHeader()
    : m_packetLength(0)
{
}

PacketHeader::~PacketHeader()
{
}

TypeId
PacketHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::sahara::PacketHeader")
                            .SetParent<Header>()
                            .SetGroupName("Sahara")
                            .AddConstructor<PacketHeader>();
    return tid;
}

TypeId
PacketHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
PacketHeader::GetSerializedSize() const
{
    return SAHARA_PKT_HEADER_SIZE;
}

void
PacketHeader::Print(std::ostream& os) const
{
    os << "len: " << m_packetLength;
}

void
PacketHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;
    i.WriteHtonU16(m_packetLength);
}

uint32_t
PacketHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    m_packetLength = i.ReadNtohU16();
    return GetSerializedSize();
}

// MessageHeader implementation

MessageHeader::MessageHeader()
    : m_messageType(HELLO_MESSAGE),
      m_reputation(0),
      m_GPS(0),
      m_battery(0),
      m_messageSize(0)
{
}

MessageHeader::~MessageHeader()
{
}

TypeId
MessageHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::sahara::MessageHeader")
                            .SetParent<Header>()
                            .SetGroupName("Sahara")
                            .AddConstructor<MessageHeader>();
    return tid;
}

TypeId
MessageHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
MessageHeader::GetSerializedSize() const
{
    uint32_t size = SAHARA_MSG_HEADER_SIZE;

    switch (m_messageType)
    {
    case HELLO_MESSAGE:
        NS_LOG_DEBUG("Hello Message Size: " << size << " + " << m_message.GetSerializedSize());
        size += m_message.GetSerializedSize();
        break;
    default:
        NS_ASSERT(false);
    }

    return size;
}

void
MessageHeader::Print(std::ostream& os) const
{
    switch (m_messageType)
    {
    case HELLO_MESSAGE:
        os << "type: HELLO";
        break;
    }

    os << " OriginIP: " << m_originIP;
    os << " hop1IP: " << m_hop1IP;
    os << " senderIP: " << m_senderIP;
    os << " rep: " << +m_reputation;
    os << " GPS: " << +m_GPS;
    os << " Battery: " << +m_battery;
    os << " Size: " << m_messageSize;

    switch (m_messageType)
    {
    case HELLO_MESSAGE:
        m_message.Print(os);
        break;
    default:
        NS_ASSERT(false);
    }
}

void
MessageHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    i.WriteU8(m_messageType);
    i.WriteHtonU32(m_originIP.Get());
    i.WriteHtonU32(m_hop1IP.Get());
    i.WriteHtonU32(m_senderIP.Get());
    i.WriteU8(m_reputation);
    i.WriteU8(m_GPS);
    i.WriteHtonU16(m_battery);

    switch (m_messageType)
    {
    case HELLO_MESSAGE:
        m_message.Serialize(i);
        break;
    default:
        NS_ASSERT(false);
    }
}

uint32_t
MessageHeader::Deserialize(Buffer::Iterator start)
{
    uint32_t size = SAHARA_MSG_HEADER_SIZE;
    Buffer::Iterator i = start;

    m_messageType = MessageType(i.ReadU8());
    NS_ASSERT(m_messageType >= HELLO_MESSAGE);
    m_originIP = Ipv4Address(i.ReadNtohU32());
    m_hop1IP = Ipv4Address(i.ReadNtohU32());
    m_senderIP = Ipv4Address(i.ReadNtohU32());
    m_reputation = i.ReadU8();
    m_GPS = i.ReadU8();
    m_battery = i.ReadNtohU16();

    switch (m_messageType)
    {
    case HELLO_MESSAGE:
        size += m_message.Deserialize(i, SAHARA_MSG_HEADER_SIZE);
        break;
    default:
        NS_ASSERT(false);
    }

    return size;
}

} // namespace sahara
} // namespace ns3
