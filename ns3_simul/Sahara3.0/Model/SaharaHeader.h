#ifndef SAHARA_HEADER_H
#define SAHARA_HEADER_H

#include "SaharaRepositories.h"

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

#include <stdint.h>
#include <vector>

namespace ns3
{
namespace olsr
{

double EmfToSeconds(uint8_t emf);
uint8_t SecondsToEmf(double seconds);

/*
  * This header only holds the common part of a message group, i.e.,
  * the first 4 bytes.
  */
class PacketHeader : public Header
{
  public:
    PacketHeader();
    ~PacketHeader() override;

    /**
     * Set the packet total length.
     * \param length The packet length.
     */
    void SetPacketLength(uint16_t length)
    {
        m_packetLength = length;
    }

    /**
     * Get the packet total length.
     * \return The packet length.
     */
    uint16_t GetPacketLength() const
    {
        return m_packetLength;
    }

    /**
     * Set the packet sequence number.
     * \param seqnum The packet sequence number.
     */
    void SetPacketSequenceNumber(uint16_t seqnum)
    {
        m_packetSequenceNumber = seqnum;
    }

    /**
     * Get the packet sequence number.
     * \returns The packet sequence number.
     */
    uint16_t GetPacketSequenceNumber() const
    {
        return m_packetSequenceNumber;
    }

  private:
    uint16_t m_packetLength;         //!< The packet length.
    uint16_t m_packetSequenceNumber; //!< The packet sequence number.

  public:

    static TypeId GetTypeId(); // always mandatorym, return the type ID of the header.

    TypeId GetInstanceTypeId() const override;
    void Print(std::ostream& os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
};

class MessageHeader : public Header
{
  public:
  /**
     * Message type
     */
    enum MessageType
    {
        HELLO_MESSAGE = 1,
       
    };

    MessageHeader();
    ~MessageHeader() override;

    /**
     * Set the message type.
     * \param messageType The message type.
     */
    void SetMessageType(MessageType messageType)
    {
        m_messageType = messageType;
    }

    /**
     * Get the message type.
     * \return The message type.
     */
    MessageType GetMessageType() const
    {
        return m_messageType;
    }

    /**
     * Set the validity time.
     * \param time The validity time.
     */
    void SetVTime(Time time)
    {
        m_vTime = SecondsToEmf(time.GetSeconds());
    }

    /**
     * Get the validity time.
     * \return The validity time.
     */
    Time GetVTime() const
    {
        return Seconds(EmfToSeconds(m_vTime));
    }

    /**
     * Set the originator address.
     * \param originatorAddress The originator address.
     */
    void SetOriginatorAddress(Ipv4Address originatorAddress)
    {
        m_originatorAddress = originatorAddress;
    }

    /**
     * Get the originator address.
     * \return The originator address.
     */
    Ipv4Address GetOriginatorAddress() const
    {
        return m_originatorAddress;
    }

    /**
     * Set the time to live.
     * \param timeToLive The time to live.
     */
    void SetTimeToLive(uint8_t timeToLive)
    {
        m_timeToLive = timeToLive;
    }

    /**
     * Get the time to live.
     * \return The time to live.
     */
    uint8_t GetTimeToLive() const
    {
        return m_timeToLive;
    }

    /**
     * Set the hop count.
     * \param hopCount The hop count.
     */
    void SetHopCount(uint8_t hopCount)
    {
        m_hopCount = hopCount;
    }

    /**
     * Get the hop count.
     * \return The hop count.
     */
    uint8_t GetHopCount() const
    {
        return m_hopCount;
    }

    /**
     * Set the message sequence number.
     * \param messageSequenceNumber The message sequence number.
     */
    void SetMessageSequenceNumber(uint16_t messageSequenceNumber)
    {
        m_messageSequenceNumber = messageSequenceNumber;
    }

    /**
     * Get the message sequence number.
     * \return The message sequence number.
     */
    uint16_t GetMessageSequenceNumber() const
    {
        return m_messageSequenceNumber;
    }

    private:
    MessageType m_messageType;
    Ipv4Address m_originIP;
    Ipv4Address m_hop1IP;
    Ipv4Address m_senderIP;
    uint8_t m_reputation;
    uint8_t m_GPS;            
    uint8_t m_battery;               
    uint16_t m_messageSize;          


    public:
    /**
     * \brief Get the type ID.
     * \return The object TypeId.
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    void Print(std::ostream& os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;

    struct Hello
    {
        /**
         * Link message item
         */
        struct LinkMessage
        {
            uint8_t linkCode; //!< Link code
            std::vector<Ipv4Address>
                neighborInterfaceAddresses; //!< Neighbor interface address container.
        };

        uint8_t hTime; //!< HELLO emission interval (coded)

        /**
         * Set the HELLO emission interval.
         * \param time The HELLO emission interval.
         */
        void SetHTime(Time time)
        {
            this->hTime = SecondsToEmf(time.GetSeconds());
        }

        /**
         * Get the HELLO emission interval.
         * \return The HELLO emission interval.
         */
        Time GetHTime() const
        {
            return Seconds(EmfToSeconds(this->hTime));
        }

        
        std::vector<LinkMessage> linkMessages; //!< Link messages container.

        /**
         * This method is used to print the content of a Hello message.
         * \param os output stream
         */
        void Print(std::ostream& os) const;
        /**
         * Returns the expected size of the header.
         * \returns the expected size of the header.
         */
        uint32_t GetSerializedSize() const;
        /**
         * This method is used by Packet::AddHeader to
         * store a header into the byte buffer of a packet.
         *
         * \param start an iterator which points to where the header should
         *        be written.
         */
        void Serialize(Buffer::Iterator start) const;
        /**
         * This method is used by Packet::RemoveHeader to
         * re-create a header from the byte buffer of a packet.
         *
         * \param start an iterator which points to where the header should
         *        read from.
         * \param messageSize the message size.
         * \returns the number of bytes read.
         */
        uint32_t Deserialize(Buffer::Iterator start, uint32_t messageSize);
    };

    private:
    /**
     * Structure holding the message content.
     */
    struct
    {
        
        Hello hello; //!< HELLO message (optional).
       
    } m_message;     //!< The actual message being carried.

    /*
     * Set the message type to HELLO and return the message content.
     * \returns The HELLO message.
     */
    Hello& GetHello()
    {
        if (m_messageType == 0)
        {
            m_messageType = HELLO_MESSAGE;
        }
        else
        {
            NS_ASSERT(m_messageType == HELLO_MESSAGE);
        }
        return m_message.hello;
    }

    const Hello& GetHello() const
    {
        NS_ASSERT(m_messageType == HELLO_MESSAGE);
        return m_message.hello;
    }
};
    inline std::ostream&
    operator<<(std::ostream& os, const PacketHeader& packet)
{
    packet.Print(os);
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const MessageHeader& message)
{
    message.Print(os);
    return os;
}

typedef std::vector<MessageHeader> MessageList;

inline std::ostream&
operator<<(std::ostream& os, const MessageList& messages)
{
    os << "[";
    for (auto messageIter = messages.begin(); messageIter != messages.end(); messageIter++)
    {
        messageIter->Print(os);
        if (messageIter + 1 != messages.end())
        {
            os << ", ";
        }
    }
    os << "]";
    return os;
}



}
}

#endif